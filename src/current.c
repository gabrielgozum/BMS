/**
 * @brief Current sensing and limits interface.
 *
 * This file includes functions to calculate SOC, DCL, and CCL.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "driverlib/sysctl.h"
#include "Lib_common.h"
#include "bms.h"
#include "current.h"
#include "charger.h"
#include "faults.h"
#include "fault_manager.h"
#include "lookup.h"
#include "ltc1864.h"
#include "params.h"
#include "scheduler.h"
#include "sdc_control.h"
#include "voltage.h"



#ifdef UNIT_TESTING
uint16_t ccl_over_sum = 0; // Sum of area above ccl
uint16_t dcl_over_sum = 0; // Sum of area above dcl
#endif


const uint16_t DISCHARGE_CURRENT_MAX = 400;  //!< [Amps]
const uint16_t CHARGE_CURRENT_MAX = 60;  //!< [Amps]
const float CURRENT_LIMIT_ALPHA = 0.04;  //!< Set for response time of 5 seconds

int32_t current_amphour_integration(float integration_period)
{
    static float last_pack_current = 0;
    float i_pack = bms_get_i_pack();
    float avg_pack_current = (last_pack_current + i_pack) / 2;
    last_pack_current = i_pack;

    return avg_pack_current * integration_period;
}


float current_lookup_soc(uint16_t ocv)
{
    // ocv is converted from 0.1 mV to V
    return lut_get_1d(&SOC_LUT, ocv / 10000.0);
}


void current_update_soc(void)
{
    bms_update_idle_state(UPDATE_SOC_PERIOD);

    if (fault_is_fVehicleIdle()
            || fault_is_cCurrentFailsafeMode()
            )
    {
        int32_t cell_i;
        for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
        {
            float soc = current_lookup_soc(bms_get_open_cell_voltage_calc(cell_i));

            if (fault_is_cCurrentFailsafeMode())
            {
                // filter since it's a CCV measurement
                float prev_soc = bms_get_cell_soc(cell_i);
                soc += SOC_FILTER_CONSTANT * (soc - prev_soc);
                bms_set_cell_capacity(cell_i, soc * PACK_NOMINAL_CAPACITY);
            }

            bms_set_cell_soc(cell_i, soc);
        }
    }
    else
    {
        // update each cell capacity
        uint8_t cell_i;
        for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
        {
            int32_t draw = current_amphour_integration(UPDATE_SOC_PERIOD);
            bms_update_cell_capacity_draw(cell_i, draw);

            float soc = draw / PACK_NOMINAL_CAPACITY;
            bms_update_cell_soc_draw(cell_i, soc);
        }
    }
}


static uint16_t current_calc_cell_temperature_ccl(void)
{
    const uint16_t CHARGE_CURRENT_TEMPERATURE_DERATE_START = 55;  //!< [deg C]
    const uint16_t CHARGE_CURRENT_DERATE = 10;  //!< [deg C / Amps]

    uint16_t current_limit;
    int8_t max_cell_temperature = bms.cell_temperature_stats.max;

    if (max_cell_temperature >= CHARGE_CURRENT_TEMPERATURE_DERATE_START)
    {
        if ((max_cell_temperature - CHARGE_CURRENT_TEMPERATURE_DERATE_START) * CHARGE_CURRENT_DERATE < CHARGE_CURRENT_MAX)
        {
            // value won't overflow, ok to proceed
            current_limit = CHARGE_CURRENT_MAX - ((max_cell_temperature - CHARGE_CURRENT_DERATE) * CHARGE_CURRENT_DERATE);
        }
        else
        {
            current_limit = 0;
        }
    }
    else
    {
        // cell temperature is OK, set limit to the maximum
        current_limit = CHARGE_CURRENT_MAX;
    }

    return current_limit;
}


void current_update_ccl(void)
{
    uint16_t charge_current_limit = 0;

    if (is_warning_active())
    {
        charge_current_limit = 0;
    }
    else
    {
        uint16_t first_order_current_limit = bms_get_cell_ch_i_limit_min();
        uint16_t temperature_current_limit = current_calc_cell_temperature_ccl();
        charge_current_limit = min_ui16(first_order_current_limit, temperature_current_limit);
    }

    float prev_charge_current_limit = bms.charge_current_limit;
    bms.charge_current_limit += CURRENT_LIMIT_ALPHA * (charge_current_limit - prev_charge_current_limit);
}


uint16_t current_calc_cell_temperature_dcl(void)
{
    const uint16_t DISCHARGE_CURRENT_TEMPERATURE_DERATE_START = 55;  //!< [deg C]
    const uint16_t DISCHARGE_CURRENT_DERATE = 80;  //!< [deg C / Amps]

    uint16_t current_limit;
    int8_t max_cell_temperature = bms.cell_temperature_stats.max;

    if (max_cell_temperature >= DISCHARGE_CURRENT_TEMPERATURE_DERATE_START)
    {
        if ((max_cell_temperature - DISCHARGE_CURRENT_TEMPERATURE_DERATE_START) * DISCHARGE_CURRENT_DERATE < DISCHARGE_CURRENT_MAX)
        {
            current_limit = DISCHARGE_CURRENT_MAX - ((max_cell_temperature - DISCHARGE_CURRENT_TEMPERATURE_DERATE_START) * DISCHARGE_CURRENT_DERATE);
        }
        else
        {
            current_limit = 0;
        }
    }
    else
    {
        current_limit = DISCHARGE_CURRENT_MAX;
    }

    return current_limit;
}


void current_update_dcl(void)
{
    uint16_t discharge_current_limit;

    if (is_warning_active() || fault_is_fChargerConnected())
    {
        discharge_current_limit = 0;
    }
    else
    {
        uint16_t ccv_cl = bms_get_cell_dch_i_limit_min();
        uint16_t ct_cl = current_calc_cell_temperature_dcl();
        discharge_current_limit = min_ui16(ccv_cl, ct_cl);
    }

    float prev_discharge_current_limit = bms.discharge_current_limit;
    bms.discharge_current_limit += CURRENT_LIMIT_ALPHA * (discharge_current_limit - prev_discharge_current_limit);
}


void current_enforce_limits(void)
{
#ifndef UNIT_TESTING
    static uint16_t ccl_over_sum = 0; // Sum of area above ccl
    static uint16_t dcl_over_sum = 0; // Sum of area above dcl
#endif

    //-------------------------------------------------------------------------
    // CCL Enforcement
    //------------------------------------------------------------------------
    float i_pack = bms_get_i_pack();

    if (-i_pack - I_MAX_ERROR > bms.charge_current_limit) // negative when charging, current is above ccl
    {
        // above limit, accumulate
        float difference = -i_pack - bms.charge_current_limit; // find the amound
        ccl_over_sum += difference * CL_CHECK_PERIOD;
    }
    else
    {
        // below limit, "leak"
        if (ccl_over_sum > 0 && ccl_over_sum <= CCL_MAX_OVER)
        {
            // accumulate variable is not saturated, "leak"
            uint16_t leak = CCL_LEAK_RATE * CL_CHECK_PERIOD;
            ccl_over_sum -= ccl_over_sum < leak ? ccl_over_sum : leak;
        }
    }

    //-------------------------------------------------------------------------
    // DCL Enforcement
    //-------------------------------------------------------------------------
    if (i_pack - I_MAX_ERROR > bms.discharge_current_limit) // current is above dcl
    {
        // above limit, accumulate
        float difference = i_pack - bms.discharge_current_limit; // find the amount
        dcl_over_sum += difference * CL_CHECK_PERIOD; // convert to A * ms
    }
    else
    {
        // below limit, "leak"
        if (dcl_over_sum > 0 && dcl_over_sum <= DCL_MAX_OVER)
        {
            // accumulate variable is not saturated, "leak"
            uint16_t leak = DCL_LEAK_RATE * CL_CHECK_PERIOD;
            dcl_over_sum -= dcl_over_sum < leak ? dcl_over_sum : leak;
        }
    }

    //-------------------------------------------------------------------------
    // Should we open the powerstage
    //-------------------------------------------------------------------------

    if (bms.charge_current_limit == 0
            && bms.discharge_current_limit == 0
            && i_pack < I_MAX_ERROR)
    {
        sdc_control_open(); // powerstage_open gets set to true
    }

    if (dcl_over_sum > DCL_MAX_OVER)
    {
        // cumulative over limit has been reached
        sdc_control_open();
        fault_set_wDischargeCurrentLimitOver(bms.discharge_current_limit, i_pack);  // doesn't clear
    }

    if (ccl_over_sum > CCL_MAX_OVER)
    {
        // cumulative over limit has been reached
        sdc_control_open();
        fault_set_wChargeCurrentLimitOver(bms.charge_current_limit, i_pack);  // doesn't clear
    }

    //-------------------------------------------------------------------------
    // Powerstage failed, non-recoverable fault
    //-------------------------------------------------------------------------
    if (fault_is_aPowerStageOpen()
            && fabs(i_pack) > I_MAX_ERROR)
    {
        fault_set_wPowerStageFailure();  // doesn't clear
    }
}


float current_get_measurement(void)
{
    float voltage = ltc1864_get_measurement();
    bms.i_pack_voltage = voltage;
    float i_measured = 0;

    if (voltage < I_SENSE_OORL_VOLTAGE)
    {
        fault_set_cCurrentSensorOORL(voltage);
    }
    else if (voltage > I_SENSE_OORH_VOLTAGE)
    {
       fault_set_cCurrentSensorOORH(voltage);
    }
    else
    {
       // Voltage is valid. Convert to I value. Apply drift.
       i_measured = (voltage - 2.5) * 1000.0 / I_SENSE_GAIN;
       i_measured = i_measured - bms_get_i_pack_offset();

       // clear fault if one existed
       fault_clear_cCurrentSensorOORH();
       fault_clear_cCurrentSensorOORL();
    }

    return i_measured;
}


float current_calc_offset(void)
{
    float i_offset = 0;

    int32_t i;
    for (i = 0; i < OFFSET_AVERAGE_SAMPLES; i++)
    {
        current_start_measurement();
        // Wait to be ready for sample
        SysCtlDelay(800 / 3); // wait ~10us
        float i_measured = current_get_measurement();

        // lpf
        i_offset = i_offset * 0.9 + i_measured * 0.1;
    }

    return i_offset;
}


void current_start_measurement(void)
{
    ltc1864_start_measurement();
}
