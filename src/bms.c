/**
 * @brief BMS Application Interface
 *
 * This file includes functions to interact with the BMS Application
 */


#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "bms.h"
#include "params.h"
#include "eeprom.h"
#include "faults.h"
#include "fault_manager.h"
#include "lookup.h"
#include "ltc1864.h"
#include "ltc6811.h"
#include "temperature.h"
#include "Lib_common.h"

extern cell_asic bms_ic[];

static const uint8_t c_i_lut[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; //!< LUT for ID conversions

bms_t bms;


static void bms_stats_reset(stats_t *s)
{
    s->average = 0;
    s->invalid_count = 0;
    s->max = 0;
    s->max_id = 0;
    s->min = 0;
    s->min_id = 0;
}


static void bms_cells_reset(cell_t *cells)
{
    uint8_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        cells[cell_i].balancing_active = 0;
        cells[cell_i].temperature = 20;  // assumed because there isn't a startup temperature measurement
        cells[cell_i].closed_cell_voltage = 0;
        cells[cell_i].open_cell_voltage = 0;
        cells[cell_i].charge_res = 0;
        cells[cell_i].discharge_res = 0;
    }
}


static void bms_bms_init(bms_t *b)
{
    // i information
    b->i_pack_offset = 0;
    b->discharge_current_limit = 0;
    b->charge_current_limit = 0;
    b->i_pack[0] = 0;

    // Pack stats
    bms_stats_reset(&(b->discharge_res_stats));
    bms_stats_reset(&(b->charge_res_stats));
    bms_stats_reset(&(b->open_cell_voltage_stats));
    bms_stats_reset(&(b->closed_cell_voltage_stats));
    bms_stats_reset(&(b->cell_temperature_stats));

    // flags
    b->airs_closed = false;

    // EEPROM stats
    // eeprom_stats_load((&b->stored_stats));
    // b->eeprom_write_ctr = 0;

    b->cell_balancing_commanded = false;
    b->num_cells_balancing = 0;
    b->balancing_vltg_limit = 38300;

    bms_set_charging_limits(&(b->limits));

    bms_cells_reset(b->cells);
}


void bms_init(void)
{
    bms_bms_init(&bms);
}


// TODO: edit this for single temp setup
void bms_cell_to_bsb_id_ct(uint8_t cell_id, uint8_t *ic_id, uint8_t *c_i)
{
    if (cell_id < TOTAL_CELLS)
    {
        *ic_id = (cell_id / CELLS_PER_BSB); // temp ic is first
        *c_i = c_i_lut[cell_id % CELLS_PER_BSB];
    }
    else
    {
        *ic_id = 0xff;
        *c_i = 0xff;
    }
}


void bms_cell_to_bsb_id_ccv(uint8_t cell_id, uint8_t *ic_id, uint8_t *c_i)
{
    if (cell_id < TOTAL_CELLS)
    {
        *ic_id = (cell_id / CELLS_PER_BSB);
        *c_i = c_i_lut[cell_id % CELLS_PER_BSB];
    }
    else
    {
        *ic_id = 0xff;
        *c_i = 0xff;
    }
}


void bms_ch_to_cell_ids(uint8_t ch, uint8_t ic_id, uint8_t cell_array[])
{
    uint8_t offset_1 = 0;
    uint8_t offset_2 = 0;
    switch(ch)
    {
    case CELL_CH_1and7:
        offset_1 = 0; // Cell 1 is id 0. From datasheet, page 59
        offset_2 = 5;
        break;
    case CELL_CH_2and8:
        offset_1 = 1;
        offset_2 = 6;
        break;
    case CELL_CH_3and9:
        offset_1 = 2;
        offset_2 = 7;
        break;
    case CELL_CH_4and10:
        offset_1 = 3;
        offset_2 = 8;
        break;
    case CELL_CH_5and11:
        offset_1 = 4;
        offset_2 = 9;
        break;
    case CELL_CH_6and12: // Ignored in Cell IDs
        offset_1 = 5;
        offset_2 = 10;
    default:
        offset_1 = 0xff;
        offset_2 = 0xff;
        break;
    }

    if (offset_1 < CELLS_PER_BSB && offset_2 < CELLS_PER_BSB && ic_id < TOTAL_ICS)
    {
        // uint8_t ic_offset = (ic_id / 2) * CELLS_PER_BSB; // Temperature ic will round down, so cell ids are the same
        uint8_t ic_offset = ic_id * CELLS_PER_BSB; // Temperature ic will round down, so cell ids are the same
        cell_array[0] = ic_offset + offset_1;
        cell_array[1] = ic_offset + offset_2;
    }
    else
    {
        cell_array[0] = 0xff;
        cell_array[1] = 0xff;
    }
}


void bms_lookup_cell_res(void)
{
    uint8_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        int8_t temperature = bms_get_cell_temperature(cell_i);
        float soc = bms.cells[cell_i].soc;

        float ch_res = lut_get_2d(&CR_CH_LUT, soc, (float)temperature);
        bms.cells[cell_i].charge_res = (uint16_t)(ch_res * 10000.0 + 0.5);

        float dch_res = lut_get_2d(&CR_DCH_LUT, soc, (float)temperature);
        bms.cells[cell_i].discharge_res = (uint16_t)(dch_res * 10000.0 + 0.5);
    }
}


void bms_set_driving_limits(limits_t *l)
{
    l->closed_cell_voltage_max = CELL_VOLTAGE_MAX_LIMIT;
    l->closed_cell_voltage_min = CELL_VOLTAGE_MIN_LIMIT;
}


void bms_set_charging_limits(limits_t *l)
{
    l->closed_cell_voltage_max = CELL_VOLTAGE_MAX_LIMIT;
    l->closed_cell_voltage_min = CELL_VOLTAGE_MIN_LIMIT;
}


void bms_set_i_failsafe(bool active)
{
    if (active)
    {
        fault_set_cCurrentFailsafeMode();
        bms.limits.closed_cell_voltage_max = CELL_VOLTAGE_MAX_FAILSAFE_LIMIT;
        bms.limits.closed_cell_voltage_min = CELL_VOLTAGE_MIN_FAILSAFE_LIMIT;
    }
    else
    {
        fault_clear_cCurrentFailsafeMode();

        if (!fault_is_cVoltageFailsafeMode())
        {
            bms.limits.closed_cell_voltage_max = CELL_VOLTAGE_MAX_LIMIT;
            bms.limits.closed_cell_voltage_min = CELL_VOLTAGE_MIN_LIMIT;
        }
    }
}


void bms_set_voltage_failsafe(bool active)
{
    if (active)
    {
        fault_set_cVoltageFailsafeMode();
        bms.limits.closed_cell_voltage_max = CELL_VOLTAGE_MAX_FAILSAFE_LIMIT;
        bms.limits.closed_cell_voltage_min = CELL_VOLTAGE_MIN_FAILSAFE_LIMIT;
    }
    else
    {
        fault_clear_cVoltageFailsafeMode();

        if (!fault_is_cCurrentFailsafeMode())
        {
            bms.limits.closed_cell_voltage_max = CELL_VOLTAGE_MAX_LIMIT;
            bms.limits.closed_cell_voltage_min = CELL_VOLTAGE_MIN_LIMIT;
        }
    }
}

void bms_update_idle_state(uint16_t update_period)
{
    static uint32_t idle_timer = 0; // Stores time car is idle (no current flow)

    if (fabs(bms_get_i_pack()) < I_MAX_ERROR
            && !fault_is_cCurrentFailsafeMode())
    {
        if(idle_timer >= IDLE_TIME_LIM * 1000 / update_period)
        {
            fault_set_fVehicleIdle();
        }
        else
        {
            idle_timer++;
        }
    }
    else
    {
        idle_timer = 0;
        fault_clear_fVehicleIdle();
    }
}


static void bms_calc_ccv_stats(void)
{
    // local vars to hold results of the calculations
    uint32_t max = 0;
    uint32_t max_id = 0;
    uint32_t min = 0xffffffff;
    uint32_t min_id = 0;
    uint32_t total = 0;
    uint32_t num_valid_cells = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        if (is_cell_voltage_valid(cell_id))
        {
            num_valid_cells++;

            uint16_t ccv = bms.cells[cell_id].closed_cell_voltage;

            // check minimum
            if (ccv < min)
            {
                min = ccv;
                min_id = cell_id;
            }

            // check maximum
            if (ccv > max)
            {
                max = ccv;
                max_id = cell_id;
            }

            // accumulate for average and ccv_pack
            total += ccv;
        }
    }

    // place the values into the bms struct for use elsewhere
    bms.closed_cell_voltage_stats.max = (uint16_t)max;
    bms.closed_cell_voltage_stats.max_id = (uint8_t)max_id;
    bms.closed_cell_voltage_stats.min = (uint16_t)min;
    bms.closed_cell_voltage_stats.min_id = (uint8_t)min_id;
    bms.pack_ccv = (uint16_t)(total / 100); // LSB = 0.01 Volts
    bms.closed_cell_voltage_stats.invalid_count = TOTAL_CELLS - num_valid_cells;

    if (num_valid_cells == 0)
    {
        bms.closed_cell_voltage_stats.average = 0;
    }
    else
    {
        bms.closed_cell_voltage_stats.average = (uint16_t)(total / num_valid_cells);
    }
}


static void bms_calc_ocv_stats(void)
{
    uint32_t max = 0;
    uint32_t max_id = 0;
    uint32_t min = 0xffffffff;
    uint32_t min_id = 0;
    uint32_t total = 0;
    uint32_t num_valid_cells = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        if (is_cell_voltage_valid(cell_id))
        {
            num_valid_cells++;

            uint16_t ocv = bms.cells[cell_id].open_cell_voltage;

            // check minimum
            if (ocv < min)
            {
                min = ocv;
                min_id = cell_id;
            }

            // check maximum
            if (ocv > max)
            {
                max = ocv;
                max_id = cell_id;
            }

            // accumulate for average and ccv_pack
            total += ocv;
        }
    }

    // place the values into the bms struct for use elsewhere
    bms.open_cell_voltage_stats.max = (uint16_t)max;
    bms.open_cell_voltage_stats.max_id = (uint8_t)max_id;
    bms.open_cell_voltage_stats.min = (uint16_t)min;
    bms.open_cell_voltage_stats.min_id = (uint8_t)min_id;
    bms.pack_ocv = (uint16_t)(total / 100); // LSB = 0.01 Volts

    if (num_valid_cells == 0)
    {
        bms.open_cell_voltage_stats.average = 0;
    }
    else
    {
        bms.open_cell_voltage_stats.average = (uint16_t)(total / num_valid_cells);
    }
}


static void bms_calc_charge_res_stats(void)
{
    uint32_t max = 0;
    uint32_t max_id = 0;
    uint32_t min = 0xffffffff;
    uint32_t min_id = 0;
    uint32_t total = 0;
    uint32_t num_valid_cells = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        if (is_cell_voltage_valid(cell_id))
        {
            num_valid_cells++;

            uint16_t resistance = bms.cells[cell_id].charge_res;

            // check minimum
            if (resistance < min)
            {
                min = resistance;
                min_id = cell_id;
            }

            // check maximum
            if (resistance > max)
            {
                max = resistance;
                max_id = cell_id;
            }

            // accumulate for average and ccv_pack
            total += resistance;
        }
    }

    // place the values into the bms struct for use elsewhere
    bms.charge_res_stats.max_id = (uint8_t)max_id;
    bms.charge_res_stats.min = (uint16_t)min;
    bms.charge_res_stats.max = (uint16_t)max;
    bms.charge_res_stats.min_id = (uint8_t)min_id;
    bms.pack_charge_res = (uint16_t)(total);

    if (num_valid_cells == 0)
    {
        bms.charge_res_stats.average = 0;
    }
    else
    {
        bms.charge_res_stats.average = (uint16_t)(total / num_valid_cells);
    }
}


static void bms_calc_discharge_res_stats(void)
{
    uint32_t max = 0;
    uint32_t max_id = 0;
    uint32_t min = 0xffffffff;
    uint32_t min_id = 0;
    uint32_t total = 0;
    uint32_t num_valid_cells = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        if (is_cell_voltage_valid(cell_id))
        {
            num_valid_cells++;

            uint16_t resistance = bms.cells[cell_id].discharge_res;

            // check minimum
            if (resistance < min)
            {
                min = resistance;
                min_id = cell_id;
            }

            // check maximum
            if (resistance > max)
            {
                max = resistance;
                max_id = cell_id;
            }

            // accumulate for average and ccv_pack
            total += resistance;
        }
    }

    // place the values into the bms struct for use elsewhere
    bms.discharge_res_stats.max_id = (uint8_t)max_id;
    bms.discharge_res_stats.min = (uint16_t)min;
    bms.discharge_res_stats.max = (uint16_t)max;
    bms.discharge_res_stats.min_id = (uint8_t)min_id;
    bms.pack_discharge_res = (uint16_t)(total);

    if (num_valid_cells == 0)
    {
        bms.discharge_res_stats.average = 0;
    }
    else
    {
        bms.discharge_res_stats.average = (uint16_t)(total / num_valid_cells);
    }
}


static void bms_calc_cell_temperature_stats(void)
{
    int32_t max = 0;
    uint32_t max_id = 0;
    int32_t min = 0x7fffffff;
    uint32_t min_id = 0;
    int32_t total = 0;
    uint32_t num_valid_cells = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        if (is_cell_temperature_valid(cell_id))
        {
            num_valid_cells++;

            int32_t temperature = bms.cells[cell_id].temperature;

            // check minimum
            if (temperature < min)
            {
                min = temperature;
                min_id = cell_id;
            }

            // check maximum
            if (temperature > max)
            {
                max = temperature;
                max_id = cell_id;
            }

            // accumulate for average and ccv_pack
            total += temperature;
        }
    }

    // place the values into the bms struct for use elsewhere
    bms.cell_temperature_stats.max_id = (uint8_t)max_id;
    bms.cell_temperature_stats.min = (int8_t)min;
    bms.cell_temperature_stats.max = (int8_t)max;
    bms.cell_temperature_stats.min_id = (uint8_t)min_id;

    if (num_valid_cells == 0)
    {
        bms.cell_temperature_stats.average = 0;
    }
    else
    {
        bms.cell_temperature_stats.average = (int8_t)(total / num_valid_cells);
    }
}


static void bms_calc_cell_soc_stats(void)
{
    float max = -10;
    uint32_t max_id = 0;
    float min = 10;
    uint32_t min_id = 0;
    float total = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        float soc = bms.cells[cell_id].soc;

        // check minimum
        if (soc < min)
        {
            min = soc;
            min_id = cell_id;
        }

        // check maximum
        if (soc > max)
        {
            max = soc;
            max_id = cell_id;
        }

        // accumulate for average and ccv_pack
        total += soc;
    }

    // place the values into the bms struct for use elsewhere
    bms.cell_soc_stats.max_id = max_id;
    bms.cell_soc_stats.min = min * 1000;
    bms.cell_soc_stats.max = max * 1000;
    bms.cell_soc_stats.min_id = min_id;
    bms.cell_soc_stats.average = total * 1000 / TOTAL_CELLS;
}


static void bms_calc_cell_capacity_stats(void)
{
    int32_t max = 0;
    uint32_t max_id = 0;
    int32_t min = 0x7fffffff;
    uint32_t min_id = 0;
    uint32_t average = 0;

    uint32_t cell_id;
    for (cell_id = 0; cell_id < TOTAL_CELLS; cell_id++)
    {
        int32_t capacity = bms.cells[cell_id].capacity;

        // check minimum
        if (capacity < min)
        {
            min = capacity;
            min_id = cell_id;
        }

        // check maximum
        if (capacity > max)
        {
            max = capacity;
            max_id = cell_id;
        }

        // accumulate for average and ccv_pack
        average += capacity / TOTAL_CELLS;
    }

    // place the values into the bms struct for use elsewhere
    bms.cell_capacity_stats.max_id = max_id;
    bms.cell_capacity_stats.min = min;
    bms.cell_capacity_stats.max = max;
    bms.cell_capacity_stats.min_id = min_id;
    bms.cell_capacity_stats.average = average;
}


static void bms_calc_first_order_limit(void)
{
    int32_t ch_max = 0;
    uint32_t ch_max_id = 0;
    int32_t ch_min = 0x7fffffff;
    uint32_t ch_min_id = 0;
    int32_t ch_total = 0;

    int32_t dch_max = 0;
    uint32_t dch_max_id = 0;
    int32_t dch_min = 0x7fffffff;
    uint32_t dch_min_id = 0;
    int32_t dch_total = 0;

    uint32_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        uint16_t ocv = lut_get_1d(&OCV_LUT, bms.cells[cell_i].soc) * 1000 * 10;
        uint16_t ccv = bms_get_closed_cell_voltage_calc(cell_i);
        int32_t v_diff;
        int32_t i_limit;

        // calc ccl
        v_diff = (int32_t)bms.limits.closed_cell_voltage_max - (int32_t)ocv;
        i_limit = v_diff / bms.cells[cell_i].charge_res;
        bms.cells[cell_i].ch_i_limit = sat_i32(i_limit, 0, 255);

        // calc dcl
        v_diff = (int32_t)ocv - (int32_t)bms.limits.closed_cell_voltage_min;
        i_limit = v_diff / bms.cells[cell_i].discharge_res;
        bms.cells[cell_i].dch_i_limit = sat_i32(i_limit, 0, 400);

        // ch stats
        int32_t ch_i_limit = bms.cells[cell_i].ch_i_limit;

        if (ch_i_limit < ch_min)
        {
            ch_min = ch_i_limit;
            ch_min_id = cell_i;
        }

        if (ch_max > ch_max)
        {
            ch_max = ch_i_limit;
            ch_max_id = cell_i;
        }

        ch_total += ch_i_limit;

        // dch stats
        int32_t dch_i_limit = bms.cells[cell_i].dch_i_limit;

        if (dch_i_limit < dch_min)
        {
            dch_min = dch_i_limit;
            dch_min_id = cell_i;
        }

        if (dch_max > dch_max)
        {
            dch_max = dch_i_limit;
            dch_max_id = cell_i;
        }

        dch_total += dch_i_limit;
    }

    // place the values into the bms struct for use elsewhere
    bms.cell_ch_i_limit_stats.max_id = ch_max_id;
    bms.cell_ch_i_limit_stats.max = ch_max;
    bms.cell_ch_i_limit_stats.min_id = ch_min_id;
    bms.cell_ch_i_limit_stats.min = ch_min;
    bms.cell_ch_i_limit_stats.average = ch_total / TOTAL_CELLS;

    bms.cell_dch_i_limit_stats.max_id = dch_max_id;
    bms.cell_dch_i_limit_stats.max = dch_max;
    bms.cell_dch_i_limit_stats.min_id = dch_min_id;
    bms.cell_dch_i_limit_stats.min = dch_min;
    bms.cell_dch_i_limit_stats.average = dch_total / TOTAL_CELLS;
}


void bms_calc_cell_stats(void)
{
    bms_calc_ccv_stats();
    bms_calc_ocv_stats();
    bms_calc_cell_soc_stats();
    bms_calc_cell_capacity_stats();
    bms_lookup_cell_res();  // Must calc before resistance stats
    bms_calc_first_order_limit();
    bms_calc_cell_temperature_stats();
    bms_calc_charge_res_stats();
    bms_calc_discharge_res_stats();
}


void bms_transfer_cell_temperature(uint8_t channel)
{
    int8_t ic_i = 0;
    for (ic_i = 0; ic_i < TOTAL_ICS; ic_i = ic_i + 1)
    {
        //TODO: not sure if this channel is correct....
        uint8_t cell_i = ic_i * CELLS_PER_BSB + channel;

        bms.cells[cell_i].temperature = temperature_calc_from_aux(cell_i);
    }
}


void bms_check_for_voltage_faults(void)
{
    uint8_t cell_i = 0;
    for(cell_i = 0; cell_i < TOTAL_CELLS; cell_i++ )
    {
        uint16_t cell_voltage = bms.cells[cell_i].closed_cell_voltage;

        if(cell_voltage > CELL_VOLTAGE_OORH_LIMIT)
        {
            set_cell_voltage_oorh_fault(cell_i, cell_voltage);
        }
        else if(cell_voltage > bms.limits.closed_cell_voltage_max)
        {
            set_cell_overvoltage_fault(cell_i, cell_voltage);
        }
        else if (cell_voltage < CELL_VOLTAGE_OORL_LIMIT)
        {
            set_cell_voltage_oorl_fault(cell_i, cell_voltage);
        }
        else if(cell_voltage < bms.limits.closed_cell_voltage_min)
        {
            set_cell_undervoltage_fault(cell_i, cell_voltage);
        }
        else
        {
            clear_cell_overvoltage_fault(cell_i);
            clear_cell_undervoltage_fault(cell_i);
            clear_cell_voltage_oorh_fault(cell_i);
            clear_cell_voltage_oorl_fault(cell_i);
        }
    }
}


uint16_t bms_get_cell_voltage_measurement(uint8_t cell_i)
{
    uint8_t ic_i;
    uint8_t c_i;

    bms_cell_to_bsb_id_ccv(cell_i, &ic_i, &c_i);

    uint16_t offset;

    switch (cell_i)
    {
    /*
    EXAMPLE
    case 9:
        offset = 360;
        break;
    */
    default:
        offset = 0;
        break;
    }

    return ltc6811_get_c_code(ic_i, c_i) + offset;
}


void bms_transfer_cell_voltage(uint8_t cell_i)
{
    uint16_t v_measured = bms_get_cell_voltage_measurement(cell_i);
    
    bms.cells[cell_i].closed_cell_voltage = v_measured;

    float i_pack = bms_get_i_pack();
    uint16_t resistance = (i_pack > 0) ? bms.cells[cell_i].discharge_res : bms.cells[cell_i].charge_res;
    bms.cells[cell_i].open_cell_voltage = v_measured - i_pack * resistance;
}


void bms_transfer_cell_voltages(uint8_t cell_group)
{
    uint8_t ic_i;
    for (ic_i = 0; ic_i < TOTAL_ICS; ic_i = ic_i + 1) //TODO: changed ic_i + 2 to + 1
    {
        uint8_t cell_i = ic_i * CELLS_PER_BSB;
        if (cell_group == 1)
        {
            bms_transfer_cell_voltage(cell_i + 0);
            bms_transfer_cell_voltage(cell_i + 1);
            bms_transfer_cell_voltage(cell_i + 2);
        }
        else if (cell_group == 2)
        {
            bms_transfer_cell_voltage(cell_i + 3);
            bms_transfer_cell_voltage(cell_i + 4);
        }
        else if (cell_group == 3)
        {
            bms_transfer_cell_voltage(cell_i + 5);
            bms_transfer_cell_voltage(cell_i + 6);
            bms_transfer_cell_voltage(cell_i + 7);
        }
        else if (cell_group == 4)
        {
            bms_transfer_cell_voltage(cell_i + 8);
            bms_transfer_cell_voltage(cell_i + 9);
            bms_transfer_cell_voltage(cell_i + 10);
        }
        else
        {

        }
    }
}

//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

float bms_get_i_pack(void)
{
    return bms.i_pack[0];
}


float bms_get_i_pack_offset(void)
{
    return bms.i_pack_offset;
}


float bms_get_i_pack_voltage(void)
{
    return bms.i_pack_voltage;
}


uint16_t bms_get_closed_cell_voltage(uint8_t cell_i)
{
    return bms.cells[cell_i].closed_cell_voltage;
}


uint16_t bms_get_closed_cell_voltage_calc(uint8_t cell_i)
{
    uint16_t cell_v;

    if (is_cell_voltage_valid(cell_i))
    {
        cell_v = bms.cells[cell_i].closed_cell_voltage;
    }
    else
    {
        cell_v = bms.closed_cell_voltage_stats.average;
    }

    return cell_v;
}


uint16_t bms_get_open_cell_voltage(uint8_t cell_i)
{
    return bms.cells[cell_i].open_cell_voltage;
}


uint16_t bms_get_open_cell_voltage_calc(uint8_t cell_i)
{
    uint16_t cell_v;

    if (is_cell_voltage_valid(cell_i))
    {
        cell_v = bms.cells[cell_i].open_cell_voltage;
    }
    else
    {
        cell_v = bms.open_cell_voltage_stats.average;
    }

    return cell_v;
}


int8_t bms_get_cell_temperature(uint8_t cell_i)
{
    return bms.cells[cell_i].temperature;
}


uint32_t bms_get_cell_capacity(uint8_t cell_i)
{
    return bms.cells[cell_i].capacity;
}


float bms_get_cell_soc(uint8_t cell_i)
{
    return bms.cells[cell_i].soc;
}


uint8_t bms_get_cell_soc_max_id(void)
{
    return (uint8_t)bms.cell_soc_stats.max_id;
}


float bms_get_cell_soc_max(void)
{
    return (float)bms.cell_soc_stats.max / 1000.0;
}


uint8_t bms_get_cell_soc_min_id(void)
{
    return (uint8_t)bms.cell_soc_stats.min_id;
}


float bms_get_cell_soc_min(void)
{
    return (float)bms.cell_soc_stats.min / 1000.0;
}


float bms_get_cell_soc_average(void)
{
    return (float)bms.cell_soc_stats.average / 1000.0;
}

uint8_t bms_get_cell_ch_i(uint8_t cell_i)
{
    return bms.cells[cell_i].ch_i_limit;
}

uint16_t bms_get_cell_dch_i(uint8_t cell_i)
{
    return bms.cells[cell_i].dch_i_limit;
}

uint8_t bms_get_cell_ch_i_limit_max_id(void)
{
    return bms.cell_ch_i_limit_stats.max_id;
}


uint16_t bms_get_cell_ch_i_limit_max(void)
{
    return bms.cell_ch_i_limit_stats.max;
}


uint8_t bms_get_cell_ch_i_limit_min_id(void)
{
    return bms.cell_ch_i_limit_stats.min_id;
}


uint16_t bms_get_cell_ch_i_limit_min(void)
{
    return bms.cell_ch_i_limit_stats.min;
}


uint16_t bms_get_cell_ch_i_limit_average(void)
{
    return bms.cell_dch_i_limit_stats.average;
}


uint8_t bms_get_cell_dch_i_limit_max_id(void)
{
    return bms.cell_dch_i_limit_stats.max_id;
}


uint16_t bms_get_cell_dch_i_limit_max(void)
{
    return bms.cell_dch_i_limit_stats.max;
}


uint8_t bms_get_cell_dch_i_limit_min_id(void)
{
    return bms.cell_dch_i_limit_stats.min_id;
}


uint16_t bms_get_cell_dch_i_limit_min(void)
{
    return bms.cell_dch_i_limit_stats.min;
}


uint16_t bms_get_cell_dch_i_limit_average(void)
{
    return bms.cell_ch_i_limit_stats.average;
}


uint8_t bms_get_cell_capacity_max_id(void)
{
    return bms.cell_capacity_stats.max_id;
}


int32_t bms_get_cell_capacity_min(void)
{
    return bms.cell_capacity_stats.min;
}


int32_t bms_get_cell_capacity_max(void)
{
    return bms.cell_capacity_stats.max;
}


uint8_t bms_get_cell_capacity_min_id(void)
{
    return bms.cell_capacity_stats.min_id;
}


int32_t bms_get_cell_capacity_average(void)
{
    return bms.cell_capacity_stats.average;
}

uint8_t bms_get_cell_charge_res(uint8_t cell_i)
{
    return bms.cells[cell_i].charge_res;
}

uint8_t bms_get_cell_discharge_res(uint8_t cell_i)
{
    return bms.cells[cell_i].discharge_res;
}

int32_t bms_get_cell_energy(uint8_t cell_i)
{
    return bms.cells[cell_i].energy;
}

float bms_get_cell_soe(uint8_t cell_i)
{
    return bms.cells[cell_i].soe;
}

bool bms_get_cell_balancing_active(uint8_t cell_i)
{
    return bms.cells[cell_i].balancing_active;
}
//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------
void bms_update_cell_capacity_draw(uint8_t cell_i, int32_t draw)
{
    bms.cells[cell_i].capacity -= draw;
}


void bms_set_cell_capacity(uint8_t cell_i, int32_t capacity)
{
    bms.cells[cell_i].capacity = capacity;
}


void bms_update_cell_soc_draw(uint8_t cell_i, float soc)
{
    bms.cells[cell_i].soc -= soc;
}


void bms_set_cell_soc(uint8_t cell_i, float soc)
{
    bms.cells[cell_i].soc = soc;
}
