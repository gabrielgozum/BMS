/**
 * @brief BMS Temperatures
 *
 * Process data from temperature sensors.
 */

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "bms.h"
#include "lookup.h"
#include "ltc6811.h"
#include "faults.h"
#include "params.h"
#include "temperature.h"


int8_t temperature_calc_from_adc(uint8_t cell_id)
{
    int8_t temperature; // stores calculated temperature
    uint8_t ic_i; // stores calculated BSB ID
    uint8_t c_i; // stores calculated BSB C ID

    bms_cell_to_bsb_id_ct(cell_id, &ic_i, &c_i);

    uint16_t cell_adc_val = ltc6811_get_c_code(ic_i, c_i);

    if (cell_adc_val <= CELL_TEMPERATURE_OORL_LIMIT)
    {
        temperature = bms.cell_temperature_stats.average;
        set_cell_temperature_oorl_fault(cell_id, cell_adc_val);
    }
    else if (cell_adc_val >= CELL_TEMPERATURE_OORH_LIMIT)
    {
        temperature = bms.cell_temperature_stats.average;
        set_cell_temperature_oorh_fault(cell_id, cell_adc_val);
    }
    else // voltage is in valid range
    {
        clear_cell_temperature_oorl_fault(cell_id);
        clear_cell_temperature_oorh_fault(cell_id);

        float voltage = cell_adc_val / 10000.0; // LSB = 100uV
        temperature = lut_get_1d(&CT_LUT, voltage) + 0.5; // round up

        if (temperature <= CELL_UNDERTEMPERATURE_LIMIT) // under temperature
        {
            set_cell_undertemperature_fault(cell_id, temperature);
        }
        else if (temperature >= CELL_OVERTEMPERATURE_LIMIT) // over temperature
        {
            set_cell_overtemperature_fault(cell_id, temperature);
        }
        else if (temperature >= 60)
        {
            set_cell_temperature_above_60_fault(cell_id, temperature);
        }
        else // temperature is valid
        {
            clear_cell_overtemperature_fault(cell_id);
            clear_cell_undertemperature_fault(cell_id);
            clear_cell_temperature_above_60_fault(cell_id);
        }
    }

    // low pass filter
    temperature = temperature * TEMP_ALPHA + bms.cells[cell_id].temperature * TEMP_BETA;

    return temperature;
}

// vref2 is 3v normally
// resistor model GA10K3A1A
// vref2 -> 10k -> 10k NTC w/ B = 3976k -> GND
// v = i/r -> r = v/i -> solve for i
// i = 3v - GPIO0 / 10k
// then calculate resistance of ntc
// then use resistance -> temperature
int8_t temperature_calc_from_aux(uint8_t cell_id)
{
    int8_t temperature; // stores calculated temperature
    uint8_t ic_i;       // stores calculated BSB ID 
    uint8_t c_i;        // stores calculated BSB C ID

    bms_cell_to_bsb_id_ct(cell_id, &ic_i, &c_i);

    // uint16_t cell_adc_val = ltc6811_get_c_code(ic_i, c_i);
    uint16_t cell_adc_val = ltc6811_get_a_code(ic_i, 0); 

    float voltage = cell_adc_val / 10000.0; // LSB = 100uV
    temperature = lut_get_1d(&CT_LUT, voltage) + 0.5; // round up

//    // todo: recalculate these values
//    if (cell_adc_val <= CELL_TEMPERATURE_OORL_LIMIT)
//    {
//        temperature = bms.cell_temperature_stats.average;
//        set_cell_temperature_oorl_fault(cell_id, cell_adc_val);
//    }
//    else if (cell_adc_val >= CELL_TEMPERATURE_OORH_LIMIT)
//    {
//        temperature = bms.cell_temperature_stats.average;
//        set_cell_temperature_oorh_fault(cell_id, cell_adc_val);
//    }
//    else // voltage is in valid range
//    {
//        clear_cell_temperature_oorl_fault(cell_id);
//        clear_cell_temperature_oorh_fault(cell_id);
//
//        float voltage = cell_adc_val / 10000.0; // LSB = 100uV
//        temperature = lut_get_1d(&CT_LUT, voltage) + 0.5; // round up
//        // float resistance = voltage / ((3-voltage)/10000);
//        // float t = (1/25) + (1/3967e3) +
//        // t =
//        // temperature = voltage * 20; //todo: temporary
//
//        if (temperature <= CELL_UNDERTEMPERATURE_LIMIT) // under temperature
//        {
//            set_cell_undertemperature_fault(cell_id, temperature);
//        }
//        else if (temperature >= CELL_OVERTEMPERATURE_LIMIT) // over temperature
//        {
//            set_cell_overtemperature_fault(cell_id, temperature);
//        }
//        else if (temperature >= 60)
//        {
//            set_cell_temperature_above_60_fault(cell_id, temperature);
//        }
//        else // temperature is valid
//        {
//            clear_cell_overtemperature_fault(cell_id);
//            clear_cell_undertemperature_fault(cell_id);
//            clear_cell_temperature_above_60_fault(cell_id);
//        }
//    }
//
   // low pass filter
    temperature = temperature * TEMP_ALPHA + bms.cells[cell_id].temperature * TEMP_BETA;
    return temperature;
}

