/**
 * @brief Charger Interface
 *
 * This file includes functions to interface with the charger
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "bms.h"
#include "can.h"
#include "charger.h"
#include "faults.h"
#include "fault_manager.h"
#include "params.h"
#include "sdc_control.h"


//! stores charging commands and received data from the charger
charger_t charger =
        {
         // Received signals
         .input_voltage = 0,
         .output_voltage = 0,
         .input_current_limit_max = 0,
         .input_current = 0,
         .output_current = 0,
         .temperature = 0,
         .ignition_status = CHARGER_IGNITION_STATUS_NA,
         .charger_state = CHARGER_STATE_NA,
         .fault_severity = CHARGER_FAULT_SEVERITY_NA,

         // Tx Signals
         .charge_enable = CHARGER_DISABLED,
         .charge_complete = CHARGER_COMPLETE_NA,
         .charge_system_fault = CHARGER_NO_FAULT,
         .voltage_limit = 0,
         .current_limit = 0,
         .time_limit = 0,
        };


void charger_process_rx_data(void)
{
    if (charger.fault_severity == CHARGER_FAULT_SEVERITY_HARD_FAIL)
    {
        fault_set_wChargerFault();

    }
    else
    {
        fault_clear_wChargerFault();
    }
}


void charger_set_command_values(void)
{
    if(fault_is_fChargerConnected()
            && !fault_is_wChargingTimeout()
            && !fault_is_wChargeCompleted()
            )
    {

        if (charger.time_limit > CHARGER_TIME_LIMIT)
        {
            fault_set_wChargingTimeout();  // doesn't clear
        }
        else
        {
            charger.time_limit += CHARGER_COMMAND_PERIOD;
        }

        uint16_t ccl = bms.charge_current_limit;

        if (ccl >= CHARGER_CURRENT_LIMIT1)
        {
            charger.voltage_limit = CHARGER_VOLTAGE_LIMIT1;
            charger.current_limit = CHARGER_CURRENT_LIMIT1;
        }
        else if (ccl >= CHARGER_CURRENT_LIMIT2)
        {
            charger.voltage_limit = CHARGER_VOLTAGE_LIMIT2;
            charger.current_limit = CHARGER_CURRENT_LIMIT2;
        }
        else
        {
            charger.voltage_limit = 0;
            charger.current_limit = 0;

            if (bms.closed_cell_voltage_stats.max > 41500) // TODO: remove hardcode
            {
                fault_set_wChargeCompleted();
            }
        }
    }
    else
    {
        // charger disabled, set limits to zero
        charger.voltage_limit = 0;
        charger.current_limit = 0;
        charger.charge_enable = CHARGER_DISABLED;
    }
}

