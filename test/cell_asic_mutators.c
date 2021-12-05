/**
 * @brief Mutators for the BMS IC struct.
 *
 * These functions can be used to simulate data being recieved from the
 * LTC6811 and put into the BMS IC struct.
 *
 * This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include <stdbool.h>
#include <stdint.h>
#include "bms.h"
#include "cell_asic_mutators.h"
#include "ltc6811.h"


/**
 * Set the cell voltage register for a cell
 *
 * @param c pointer to BMS IC struct
 * @param cell_num which cell (0 - 59)
 * @param cell_voltage voltage adc count to set cell_num to
 *
 * @returns true if successful and false if failed
 */
void cam_set_cell_voltage(cell_asic c[], uint16_t cell_voltage, uint8_t cell_num)
{
    uint32_t cell_asic_num = cell_num / 10 * 2 + 1; // voltage ic is second on bsb

    cell_num = cell_num % 10;

    switch(cell_num)
    {
        case 0:
            cell_voltage = cell_voltage + TEST_CELL_0_VOLTAGE_OFFSET;
            break;
        case 9:
            cell_voltage = cell_voltage + TEST_CELL_9_VOLTAGE_OFFSET;
            break;
        default:
            break;
    }

    if (cell_num < 4)
    {
        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
    }
    else if (cell_num == 4)
    {
        // set two registers because this cell is connected to 2 registers
        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
        c[cell_asic_num].cells.c_codes[cell_num + 1] = cell_voltage;
    }
    else if (cell_num < 9)
    {
        cell_num += 1; // offset for the cell connected to two inputs

        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
    }
    else // cell_num == 9
    {
        cell_num += 1; // offset for the cell connected to two inputs

        // set two registers because this cell is connected to 2 registers
        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
        c[cell_asic_num].cells.c_codes[cell_num + 1] = cell_voltage;
    }
}

/**
 * Set the all the cell voltage registers to the same cell_votlage
 *
 * @param c BMS IC struct
 * @param cell_voltage voltage adc count to set the cells to
 */
void cam_set_all_cell_voltages(cell_asic *c, uint16_t cell_voltage)
{
    uint8_t i;

    for (i = 0; i < TOTAL_CELLS; i++)
    {
        cam_set_cell_voltage(c, cell_voltage, i);
    }
}

/**
 * Set the cell voltage register for a cell temperature
 *
 * @param c pointer to BMS IC struct
 * @param cell_num which cell (0 - 59)
 * @param cell_voltage temperature voltage adc count to set cell_num to
 *
 * @returns true if successful and false if failed
 */
void cam_set_cell_temperature(cell_asic c[], uint16_t cell_voltage, uint8_t cell_num)
{
    uint32_t cell_asic_num = (cell_num / 10 * 2); // temperature ic is second on bsb

    cell_num = cell_num % 10;

    if (cell_num < 4)
    {
        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
    }
    else if (cell_num == 4)
    {
        // set two registers because this cell is connected to 2 registers
        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
        c[cell_asic_num].cells.c_codes[cell_num + 1] = cell_voltage;
    }
    else if (cell_num < 9)
    {
        cell_num += 1; // offset for the cell connected to two inputs

        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
    }
    else // cell_num == 9
    {
        cell_num += 1; // offset for the cell connected to two inputs

        // set two registers because this cell is connected to 2 registers
        c[cell_asic_num].cells.c_codes[cell_num] = cell_voltage;
        c[cell_asic_num].cells.c_codes[cell_num + 1] = cell_voltage;
    }
}

/**
 * Set the all the cell temperature voltage registers to the same cell_votlage
 *
 * @param c BMS IC struct
 * @param cell_voltage temperature voltage adc count to set the cells to
 */
void cam_set_all_cell_temperatures(cell_asic *c, uint16_t cell_voltage)
{
    uint8_t i;

    for (i = 0; i < TOTAL_CELLS; i++)
    {
        cam_set_cell_temperature(c, cell_voltage, i);
    }
}

void cam_set_bsbt_ratio(cell_asic c[], uint16_t ratio, uint8_t bsb_id)
{
    uint32_t ic_id = bsb_id * 2;

    c[ic_id].aux.a_codes[0] = ratio;
    c[ic_id].stat.stat_codes[2] = 99; // Set ref adc to 100, so ratio = adc value.
}

void cam_set_all_bsbt_ratios(cell_asic c[], uint16_t ratio)
{
    uint8_t i;
    
    for (i = 0; i < TOTAL_BSB; i++)
    {
        cam_set_bsbt_ratio(c, ratio, i);
    }
}

void cam_set_bsbh_ratio(cell_asic c[], uint16_t ratio, uint8_t bsb_id)
{
    uint32_t ic_id = bsb_id * 2;

    c[ic_id].aux.a_codes[1] = ratio;
    c[ic_id].stat.stat_codes[2] = 99; // Set ref adc to 100, so ratio = adc value.
}

void cam_set_all_bsbh_ratios(cell_asic c[], uint16_t ratio)
{
    uint8_t i;
    
    for (i = 0; i < TOTAL_BSB; i++)
    {
        cam_set_bsbh_ratio(c, ratio, i);
    }
}
