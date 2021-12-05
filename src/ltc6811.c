/**
 * This file includes the interface between the MCU and the BMS ICs.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "inc/hw_ints.h"
#include "inc/hw_ssi.h"
#include "bms.h"
#include "faults.h"
#include "fault_manager.h"
#include "ltc6811.h"
#include "params.h"
#include "spi.h"
#include "ltc1380.h"

// LUT for PEC15 calculation
const uint16_t crc15Table[256] = {
        0x0,
        0xc599,
        0xceab,
        0xb32,
        0xd8cf,
        0x1d56,
        0x1664,
        0xd3fd,
        0xf407,
        0x319e,
        0x3aac,  //!<precomputed CRC15 Table
        0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5,
        0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f,
        0x44c6, 0x4ff4, 0x8a6d, 0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678,
        0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d,
        0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544,
        0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5,
        0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b,
        0xc969, 0xcf0, 0xdf0d, 0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9,
        0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167,
        0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024,
        0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
        0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318,
        0xd681, 0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286,
        0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614,
        0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9, 0xf84, 0xca1d,
        0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
        0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f,
        0x21f2, 0xe46b, 0xef59, 0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5,
        0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc,
        0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
        0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b,
        0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41,
        0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x846, 0xcddf,
        0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
        0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd,
        0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0,
        0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e,
        0x450c, 0x8095 };

static bool v_dcc[12] = {false, false, false, false, false, false, false, false, false, false, false, false}; //!< State of voltage discharge enable
extern cell_asic bms_ic[];

//-----------------------------------------------------------------------------
// Developed by the Wisconsin Racing Firmware Team
//-----------------------------------------------------------------------------


// TODO: delete this function
void ltc6811_enable_temperature_dcc(uint8_t channel, cell_asic ic[])
{
    bool t_dcc[12] = {false, false, false, false, false, false, false, false,
                      false, false, false, false};
    channel--;

    t_dcc[channel] = true;
    t_dcc[channel + 6] = true;

    uint8_t n_ic;
    for (n_ic = 0; n_ic < TOTAL_ICS; n_ic += 2) // only execute on temperature ICs (1st ic)
    {
        ltc6811_set_cfgr_dis(n_ic, ic, t_dcc);
    }

    wakeup_idle();
    ltc6811_wrcfg(ic);
}


uint16_t ltc6811_get_c_code(uint8_t ic_i, uint8_t c_i)
{
    // uint16_t offset;

    //TODO: this may need to change also - not sure what it does
    // if (ic_i % 2 == 1)
    // {
    //     switch(c_i)
    //     {
    //         default:
    //             offset = 0;
    //             break;
    //     }
    // }
    // else
    // {
    //     offset = 0;
    // }
    uint16_t offset = 0;
    return bms_ic[ic_i].cells.c_codes[c_i] + offset;
}

uint16_t ltc6811_get_a_code(uint8_t ic_i, uint8_t c_i)
{
    return bms_ic[ic_i].aux.a_codes[c_i];
}


void ltc6811_handle_error(uint8_t error)
{
    if (error != 0)
    {
        fault_set_wisoSPIPECError();
    }
    else
    {
        fault_clear_wisoSPIPECError();
    }
}


void ltc6811_handle_self_check(uint8_t error)
{
    if (error != 0)
    {
        fault_set_wSelfTestError();
    }
    else
    {
        fault_clear_wSelfTestError();
    }
}

/**
 * @brief Toggle the discharge pin (S) for all cells of the given
 * channel on the temperature ICs.
 *
 * @param ch The channel to change
 * @param state True if connection is on.
 * @param ic The cell_asic array.
 *
 * Evan
 */
void ltc6811_toggle_discharge_temp_ch(uint8_t ch, bool state, cell_asic ic[])
{
    static bool t_dcc[12] = {false, false, false, false, false, false, false, false, false, false, false, false}; // keep states from previous calls
    uint8_t n_ic;
    for (n_ic = 1; n_ic < TOTAL_ICS; n_ic += 2) // only execute on temperature ICs (1st ic)
    {

        switch(ch)
        {
        case CELL_CH_1and7:
            t_dcc[0] = state;
            t_dcc[6] = state;
            break;
        case CELL_CH_2and8:
            t_dcc[1] = state;
            t_dcc[7] = state;
            break;
        case CELL_CH_3and9:
            t_dcc[2] = state;
            t_dcc[8] = state;
            break;
        case CELL_CH_4and10:
            t_dcc[3] = state;
            t_dcc[9] = state;
            break;
        case CELL_CH_5and11:
            t_dcc[4] = state;
            t_dcc[10] = state;
            break;
        case CELL_CH_6and12:
            t_dcc[5] = state;
            t_dcc[11] = state;
            break;
        default:
            break;
        }

        ltc6811_set_cfgr_dis(n_ic, ic, t_dcc);
    }

    wakeup_idle();
    ltc6811_wrcfg(ic);
}

/**
 * @brief Set the discharge pins on temperature ICs to the given array.
 * Not compatible with ltc6811_toggle_discharge_temp_ch
 *
 * @param dcc Array of dcc values for each IC, length of 12 elements
 * @param state True if connection is on.
 * @param ic The cell_asic array.
 *
 * Evan
 */
void ltc6811_set_discharge_temp(bool *dcc, bool state, cell_asic ic[])
{
    uint8_t n_ic;
    for (n_ic = 0; n_ic < TOTAL_ICS; n_ic += 2) // only execute on temperature ICs (1st ic)
    {
        ltc6811_set_cfgr_dis(n_ic, ic, dcc);
    }
    wakeup_idle();
    ltc6811_wrcfg(ic);
}

/**
 * @brief Toggle the discharge pin (S) for all cells of the given
 * channel on the voltage ICs.
 *
 * @param ch The channel to change
 * @param state True if connection is on.
 * @param ic The cell_asic array.
 *
 * Evan
 */
void ltc6811_toggle_discharge_vltg_ch(uint8_t ch, bool state, cell_asic ic[])
{
    uint8_t n_ic;
    for (n_ic = 1; n_ic < TOTAL_ICS; n_ic += 2) // only execute on voltage ic's (2nd ic)
    {

        switch(ch)
        {
        case CELL_CH_1and7:
            v_dcc[0] = state;
            v_dcc[6] = state;
            break;
        case CELL_CH_2and8:
            v_dcc[1] = state;
            v_dcc[7] = state;
            break;
        case CELL_CH_3and9:
            v_dcc[2] = state;
            v_dcc[8] = state;
            break;
        case CELL_CH_4and10:
            v_dcc[3] = state;
            v_dcc[9] = state;
            break;
        case CELL_CH_5and11:
            v_dcc[4] = state;
            v_dcc[10] = state;
            break;
        case CELL_CH_6and12:
            v_dcc[5] = state;
            v_dcc[11] = state;
            break;
        default:
            break;
        }

        ltc6811_set_cfgr_dis(n_ic, ic, v_dcc);
    }

    wakeup_idle();
    ltc6811_wrcfg(ic);
}

/**
 * @brief Toggle the discharge pin (S) for a given cell
 *
 * @param cell The cell to change
 * @param state True if connection is on.
 * @param exclusive True to clear all other active discharge gates
 * @param ic The cell_asic array.
 *
 * Evan
 */
void ltc6811_toggle_discharge_vltg_cell(uint8_t cell, bool state, bool exclusive, cell_asic ic[])
{
    if (exclusive)
    {
        uint8_t i;
        for (i = 0; i < 12; i++)
        {
            v_dcc[i] = false;
        }
    }
    uint8_t ic_id = 0;
    uint8_t c_id = 1;
    bms_cell_to_bsb_id_ccv(cell, &ic_id, &c_id);

    v_dcc[c_id] = state;

    ltc6811_set_cfgr_dis(ic_id, ic, v_dcc);
}

// /**
//  * Reads and parses the ltc681x cell voltage registers.
//  * Throws out temperature reading.
//  *
//  * @param controls which cell voltage register is read back
//  * @param array of the parsed cell codes
//  *
//  * Evan
//  */
// uint8_t ltc6811_rdcv_voltage(uint8_t reg, cell_asic ic[])
// {
//     int8_t pec_error = 0;
//     uint8_t cell_data[NUM_RX_BYT * TOTAL_ICS];
//     uint8_t c_ic = 0;

//     if (reg == 0)
//     {
//         uint32_t cell_reg = 0;
//         for (cell_reg = 1; cell_reg < ic[0].ic_reg.num_cv_reg + 1; cell_reg++) //executes once for each of the ltc6811 cell voltage registers
//         {
//             ltc6811_rdcv_reg(cell_reg, cell_data);

//             uint32_t current_ic = 0;
//             for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++) // voltage ic
//             {
//                 if (ic->isospi_reverse == false)
//                 {
//                     c_ic = current_ic;
//                 }
//                 else
//                 {
//                     c_ic = TOTAL_ICS - current_ic - 1;
//                 }
//                 pec_error = pec_error
//                         + parse_cells(current_ic, cell_reg, cell_data,
//                                       &ic[c_ic].cells.c_codes[0],
//                                       &ic[c_ic].cells.pec_match[0]);
//             }
//         }
//     }

//     else
//     {
//         ltc6811_rdcv_reg(reg, cell_data);

//         uint32_t current_ic = 0;
//         for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++) // voltage
//         {
//             if (ic->isospi_reverse == false)
//             {
//                 c_ic = current_ic;
//             }
//             else
//             {
//                 c_ic = TOTAL_ICS - current_ic - 1;
//             }
//             pec_error = pec_error
//                     + parse_cells(current_ic, reg, &cell_data[8 * c_ic],
//                                   &ic[c_ic].cells.c_codes[0],
//                                   &ic[c_ic].cells.pec_match[0]);
//         }
//     }
//     ltc6811_check_pec(CELL, ic);
//     return (pec_error);
// }

// //TODO: delete

// /*
// The function is used to read the  parsed GPIO codes of the LTC681x. 
// This function will send the requested read commands parse the data 
// and store the gpio voltages in a_codes variable.
// */
// uint8_t ltc6811_rdaux_temp(uint8_t reg, cell_asic ic[])
// {
// 	int8_t pec_error = 0;
// 	uint8_t c_ic =0;
//     uint8_t cell_data[NUM_RX_BYT * TOTAL_ICS];

// 	if (reg == 0)
// 	{
// 		for (uint8_t gpio_reg = 1; gpio_reg<ic[0].ic_reg.num_gpio_reg+1; gpio_reg++) //Executes once for each of the LTC681x aux voltage registers
// 		{
// 			ltc6811_rdaux_reg(gpio_reg, cell_data);                 //Reads the raw auxiliary register data into the data[] array
// 			for (int current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
// 			{
// 				if (ic->isospi_reverse == false)
// 				{
//                     c_ic = current_ic;
// 				}
// 				else
// 				{
//                     c_ic = TOTAL_ICS - current_ic - 1;
// 				}
// 				pec_error = parse_cells(current_ic,gpio_reg, cell_data,
// 										&ic[c_ic].aux.a_codes[0],
// 										&ic[c_ic].aux.pec_match[0]);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		ltc6811_rdaux_reg(reg, cell_data);

// 		for (int current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
// 		{
// 			if (ic->isospi_reverse == false)
// 			{
//                 c_ic = current_ic;
// 			}
// 			else
// 			{
//                 c_ic = TOTAL_ICS - current_ic - 1;
// 			}
// 			pec_error = parse_cells(current_ic,reg, cell_data,
// 								  &ic[c_ic].aux.a_codes[0],
// 								  &ic[c_ic].aux.pec_match[0]);
// 		}
// 	}
// 	ltc6811_check_pec(AUX, ic);

// 	return (pec_error);
// }

// //TODO: delete, never used
// /**
//  * Reads and parses the ltc681x cell voltage registers.
//  * Throws out voltage reading.
//  *
//  * @param controls which cell voltage register is read back
//  * @param array of the parsed cell codes
//  *
//  * Evan
//  */
// uint8_t ltc6811_rdcv_temperature(uint8_t reg, cell_asic ic[])
// {
//     int8_t pec_error = 0;
//     uint8_t cell_data[NUM_RX_BYT * TOTAL_ICS];
//     uint8_t c_ic = 0;

//     if (reg == 0)
//     {
//         uint32_t cell_reg = 0;
//         for (cell_reg = 1; cell_reg < ic[0].ic_reg.num_cv_reg + 1; cell_reg++) //executes once for each of the ltc6811 cell voltage registers
//         {
//             ltc6811_rdcv_reg(cell_reg, cell_data);

//             uint32_t current_ic = 0;
//             for (current_ic = 0; current_ic < TOTAL_ICS; current_ic+=2) // temperature IC
//             {
//                 if (ic->isospi_reverse == false)
//                 {
//                     c_ic = current_ic;
//                 }
//                 else
//                 {
//                     c_ic = TOTAL_ICS - current_ic - 1;
//                 }
//                 pec_error = pec_error
//                         + parse_cells(current_ic, cell_reg, cell_data,
//                                       &ic[c_ic].cells.c_codes[0],
//                                       &ic[c_ic].cells.pec_match[0]);
//             }
//         }
//     }

//     else
//     {
//         ltc6811_rdcv_reg(reg, cell_data);

//         uint32_t current_ic = 0;
//         for (current_ic = 0; current_ic < TOTAL_ICS; current_ic+=2) // temperature
//         {
//             if (ic->isospi_reverse == false)
//             {
//                 c_ic = current_ic;
//             }
//             else
//             {
//                 c_ic = TOTAL_ICS - current_ic - 1;
//             }
//             pec_error = pec_error
//                     + parse_cells(current_ic, reg, &cell_data[8 * c_ic],
//                                   &ic[c_ic].cells.c_codes[0],
//                                   &ic[c_ic].cells.pec_match[0]);
//         }
//     }
//     ltc6811_check_pec(CELL, ic);
//     return (pec_error);
// }

//-----------------------------------------------------------------------------
// The below functions were ported over from the ltc6811 drivers.
//-----------------------------------------------------------------------------

/**
 * Generic function to write 68xx commands. Function calculated PEC for
 * tx_cmd data.
 */
void cmd_68(uint8_t tx_cmd[2])
{
    uint8_t cmd[4];
    uint16_t cmd_pec;

    cmd[0] = tx_cmd[0];
    cmd[1] = tx_cmd[1];
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_array(4, cmd, SSI1_BASE);
    setCSHigh(SSI1_BASE);
}

/**
 * Calculates  and returns the CRC15
 *
 * @param Number of bytes that will be used to calculate a PEC
 * @param Array of data that will be used to calculate  a PEC
 */
uint16_t pec15_calc(uint8_t len, uint8_t *data)
{
    uint16_t remainder = 16; // initialize the PEC
    uint16_t addr = 0;
    uint32_t i = 0;

    for (i = 0; i < len; i++) // loops for each byte in data array
    {
        addr = ((remainder >> 7) ^ data[i]) & 0xff; //calculate PEC table address
        remainder = (remainder << 8) ^ crc15Table[addr];
    }

    return (remainder * 2); //The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}

/**
 * Generic function to write 68xx commands and read data. Function calculated
 * PEC for tx_cmd data.
 */
int8_t read_68(uint8_t tx_cmd[2], uint8_t *rx_data)
{
    uint8_t cmd[4];
    uint8_t data[NUM_RX_BYT * TOTAL_ICS];
    int8_t pec_error = 0;
    uint16_t cmd_pec;
    uint16_t data_pec;
    uint16_t received_pec;

    cmd[0] = tx_cmd[0];
    cmd[1] = tx_cmd[1];
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_read(cmd, 4, data, (NUM_RX_BYT * TOTAL_ICS), SSI1_BASE); //Read the configuration data of all ICs on the daisy chain into
    setCSHigh(SSI1_BASE);                                               //rx_data[] array

    uint32_t current_ic = 0;
    for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++) //executes for each ltc681x in the daisy chain and packs the data
    {
        //into the r_comm array as well as check the received Config data
        //for any bit errors
        uint32_t current_byte = 0;
        for (current_byte = 0; current_byte < NUM_RX_BYT; current_byte++)
        {
            rx_data[(current_ic * 8) + current_byte] = data[current_byte
                    + (current_ic * NUM_RX_BYT)];
        }
        received_pec = (rx_data[(current_ic * 8) + 6] << 8)
                + rx_data[(current_ic * 8) + 7];
        data_pec = pec15_calc(6, &rx_data[current_ic * 8]);
        if (received_pec != data_pec)
        {
            pec_error = -1;
        }
    }

    return (pec_error);
}

/**
 * Helper function that parses voltage measurement registers.
 */
int8_t parse_cells(uint8_t current_ic, uint8_t cell_reg, uint8_t cell_data[],
                   uint16_t *cell_codes, uint8_t *ic_pec)
{

    const uint8_t BYT_IN_REG = 6;
    const uint8_t CELL_IN_REG = 3;
    int8_t pec_error = 0;
    uint16_t parsed_cell;
    uint16_t received_pec;
    uint16_t data_pec;
    uint8_t data_counter = current_ic * NUM_RX_BYT; //data counter

    uint32_t current_cell = 0;
    for (current_cell = 0; current_cell < CELL_IN_REG; current_cell++) // This loop parses the read back data into cell voltages, it
    {
        // loops once for each of the 3 cell voltage codes in the register

        parsed_cell = cell_data[data_counter]
                + (cell_data[data_counter + 1] << 8); //Each cell code is received as two bytes and is combined to
        // create the parsed cell voltage code
        cell_codes[current_cell + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
        data_counter = data_counter + 2; //Because cell voltage codes are two bytes the data counter
        //must increment by two for each parsed cell code
    }

    received_pec = (cell_data[data_counter] << 8) | cell_data[data_counter + 1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
    //after the 6 cell voltage data bytes
    data_pec = pec15_calc(BYT_IN_REG, &cell_data[(current_ic) * NUM_RX_BYT]);

    if (received_pec != data_pec)
    {
        pec_error = 1; //The pec_error variable is simply set negative if any PEC errors
        ic_pec[cell_reg - 1] = 1;
    }
    else
    {
        ic_pec[cell_reg - 1] = 0;
    }
    data_counter = data_counter + 2;
    return (pec_error);
}

void wakeup_idle(void)
{
    uint32_t i = 0;
    for (i = 0; i < TOTAL_ICS; i++)
    {
        setCSLow(SSI1_BASE);
        //delayMicroseconds(2); //Guarantees the isoSPI will be in ready mode
        spiTransfer(0xff, SSI1_BASE);
        setCSHigh(SSI1_BASE);
    }
}

/**
 * Generic wakeup commannd to wake the ltc6813 from sleep
 */
void wakeup_sleep(void)
{
    uint32_t i;

    for (i = 0; i < TOTAL_ICS; i++)
    {
        setCSLow(SSI1_BASE);
        SysCtlDelay(24000); // Guarantees the ltc6813 will be in standby
        setCSHigh(SSI1_BASE);
        SysCtlDelay(800);
    }
}

/**
 * Generic function to write 68xx commands and write payload data. Function
 * calculated PEC for tx_cmd data.
 */
void write_68(uint8_t tx_cmd[2], uint8_t data[])
{
    const uint8_t BYTES_IN_REG = 6;
    const uint8_t CMD_LEN = 4 + (8 * TOTAL_ICS);
    uint8_t cmd[4 + (8 * TOTAL_ICS)];
    uint16_t data_pec;
    uint16_t cmd_pec;
    uint8_t cmd_index;

    cmd[0] = tx_cmd[0];
    cmd[1] = tx_cmd[1];
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);
    cmd_index = 4;

    uint8_t current_ic = 0;
    for (current_ic = TOTAL_ICS; current_ic > 0; current_ic--) // executes for each ltc681x in daisy chain, this loops starts with
    {
        // the last IC on the stack. The first configuration written is
        // received by the last IC in the daisy chain

        uint8_t current_byte = 0;
        for (current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
        {
            cmd[cmd_index] = data[((current_ic - 1) * 6) + current_byte];
            cmd_index = cmd_index + 1;
        }

        data_pec = (uint16_t) pec15_calc(BYTES_IN_REG,
                                         &data[(current_ic - 1) * 6]); // calculating the PEC for each Iss configuration register data
        cmd[cmd_index] = (uint8_t) (data_pec >> 8);
        cmd[cmd_index + 1] = (uint8_t) data_pec;
        cmd_index = cmd_index + 2;
    }

    setCSLow(SSI1_BASE);
    spi_write_array(CMD_LEN, cmd, SSI1_BASE);
    setCSHigh(SSI1_BASE);
}

/**
 * Starts cell voltage conversion
 *
 * @param ADC Mode
 * @param Discharge Permit
 * @param Cell Channels to be measured
 *
 */
void ltc6811_adcv(uint8_t MD, uint8_t DCP, uint8_t CH)
{
  uint8_t cmd[4];
  uint8_t md_bits;

  md_bits = (MD & 0x02) >> 1;
  cmd[0] = md_bits + 0x02;
  md_bits = (MD & 0x01) << 7;
  cmd[1] =  md_bits + 0x60 + (DCP<<4) + CH;
  cmd_68(cmd);
}

/**
 * Start a GPIO and Vref2 Conversion
 *
 * @param ADC Mode
 * @param GPIO Channels to be measured
 *
 */
void ltc6811_adax(uint8_t MD, uint8_t CHG)
{
  uint8_t cmd[4];
  uint8_t md_bits;

  md_bits = (MD & 0x02) >> 1;
  cmd[0] = md_bits + 0x04;
  md_bits = (MD & 0x01) << 7;
  cmd[1] = md_bits + 0x60 + CHG ;
  cmd_68(cmd);
}


/**
 * Start a cell voltage and GPIO 1/2 conversion
 *
 * @param ADC Mode
 * @param Discharge Permit
 *
 */
void ltc6811_adcvax(uint8_t MD, uint8_t DCP)
{
    uint8_t cmd[2];
    uint8_t md_bits;

    md_bits = (MD & 0x02) >> 1;
    cmd[0] = md_bits | 0x04;
    md_bits = (MD & 0x01) << 7;
    cmd[1] =  md_bits | ((DCP & 0x01) << 4) + 0x6F;

    cmd_68(cmd);
}

/**
 * Start an GPIO Redundancy test.
 *
 * @param ADC Mode
 * @param GPIO Channels to be measured
 *
 */
void ltc6811_adaxd(uint8_t MD, uint8_t CHG)
{
  uint8_t cmd[4];
  uint8_t md_bits;

  md_bits = (MD & 0x02) >> 1;
  cmd[0] = md_bits + 0x04;
  md_bits = (MD & 0x01) << 7;
  cmd[1] = md_bits + CHG ;
  cmd_68(cmd);
}

/**
 * Starts cell voltage overlap conversion
 *
 * @param ADC Mode
 * @param Discharge Permit
 */
void ltc6811_adol(uint8_t MD, uint8_t DCP)
{
    uint8_t cmd[4];
    uint8_t md_bits;

    md_bits = (MD & 0x02) >> 1;
    cmd[0] = md_bits + 0x02;
    md_bits = (MD & 0x01) << 7;
    cmd[1] = md_bits + (DCP << 4) + 0x01;

    cmd_68(cmd);
}

/**
 * Start an open wire Conversion
 *
 * @param ADC Mode
 * @param Discharge Permit
 *
 */
void ltc6811_adow(uint8_t MD, uint8_t PUP)
{
    uint8_t cmd[2];
    uint8_t md_bits;

    md_bits = (MD & 0x02) >> 1;
    cmd[0] = md_bits + 0x02;
    md_bits = (MD & 0x01) << 7;
    cmd[1] = md_bits + 0x28 + (PUP << 6); //+ CH;

    cmd_68(cmd);
}

/**
 * Start a Status ADC Conversion
 *
 * @param ADC Mode
 * @param GPIO Channels
 *
 */
void ltc6811_adstat(uint8_t MD, uint8_t CHST)
{
    uint8_t cmd[4];
    uint8_t md_bits;

    md_bits = (MD & 0x02) >> 1;
    cmd[0] = md_bits + 0x04;
    md_bits = (MD & 0x01) << 7;
    cmd[1] = md_bits + 0x68 + CHST;
    cmd_68(cmd);
}

/**
 * Start a Status register redundancy test Conversion
 *
 * @param ADC Mode
 * @param GPIO Channels to be measured
 *
 */
void ltc6811_adstatd(uint8_t MD, uint8_t CHST)
{
    uint8_t cmd[2];
    uint8_t md_bits;

    md_bits = (MD & 0x02) >> 1;
    cmd[0] = md_bits + 0x04;
    md_bits = (MD & 0x01) << 7;
    cmd[1] = md_bits + 0x08 + CHST;
    cmd_68(cmd);
}

/**
 * Start an Auxiliary Register Self Test Conversion
 *
 * @param ADC Mode
 * @param Self Test
 *
 */
void ltc6811_axst(uint8_t MD, uint8_t ST)
{
  uint8_t cmd[4];
  uint8_t md_bits;

  md_bits = (MD & 0x02) >> 1;
  cmd[0] = md_bits + 0x04;
  md_bits = (MD & 0x01) << 7;
  cmd[1] =  md_bits + ((ST&0x03)<<5) +0x07;
  cmd_68(cmd);
}

/**
 * Helper function that increments PEC counters
 *
 */
void ltc6811_check_pec(uint8_t reg, cell_asic ic[])
{
    uint32_t current_ic = 0;

    switch (reg)
    {
    case CFGR:
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
            ic[current_ic].crc_count.pec_count =
                    ic[current_ic].crc_count.pec_count
                            + ic[current_ic].config.rx_pec_match;
            ic[current_ic].crc_count.cfgr_pec =
                    ic[current_ic].crc_count.cfgr_pec
                            + ic[current_ic].config.rx_pec_match;
        }
        break;

    case CFGRB:
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
            ic[current_ic].crc_count.pec_count =
                    ic[current_ic].crc_count.pec_count
                            + ic[current_ic].configb.rx_pec_match;
            ic[current_ic].crc_count.cfgr_pec =
                    ic[current_ic].crc_count.cfgr_pec
                            + ic[current_ic].configb.rx_pec_match;
        }
        break;
    case CELL:
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
           uint32_t i = 0;
            for (i = 0; i < ic[0].ic_reg.num_cv_reg; i++)
            {
                ic[current_ic].crc_count.pec_count =
                        ic[current_ic].crc_count.pec_count
                                + ic[current_ic].cells.pec_match[i];
                ic[current_ic].crc_count.cell_pec[i] =
                        ic[current_ic].crc_count.cell_pec[i]
                                + ic[current_ic].cells.pec_match[i];
            }
        }
        break;
    case AUX:
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
            uint32_t i = 0;
            for (i = 0; i < ic[0].ic_reg.num_gpio_reg; i++)
            {
                ic[current_ic].crc_count.pec_count =
                        ic[current_ic].crc_count.pec_count
                                + (ic[current_ic].aux.pec_match[i]);
                ic[current_ic].crc_count.aux_pec[i] =
                        ic[current_ic].crc_count.aux_pec[i]
                                + (ic[current_ic].aux.pec_match[i]);
            }
        }

        break;
    case STAT:
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
            uint32_t i = 0;
            for (i = 0; i < ic[0].ic_reg.num_stat_reg - 1; i++)
            {
                ic[current_ic].crc_count.pec_count =
                        ic[current_ic].crc_count.pec_count
                                + ic[current_ic].stat.pec_match[i];
                ic[current_ic].crc_count.stat_pec[i] =
                        ic[current_ic].crc_count.stat_pec[i]
                                + ic[current_ic].stat.pec_match[i];
            }
        }
        break;
    default:
        break;
    }
}

/**
 * Clears all of the DCC bits in the configuration registers. After setting the
 * discharge bit, the ltc6811_wrcfg function must be called.
 *
 */
void ltc6811_clear_discharge(cell_asic ic[])
{
    uint32_t i = 0;
    for (i = 0; i < TOTAL_ICS; i++)
    {
        ic[i].config.tx_data[4] = 0;
        ic[i].config.tx_data[5] = 0;
    }
}

/**
 * The command clears the Auxiliary registers and initializes
 * all values to 1. The register will read back hexadecimal 0xFF
 * after the command is sent.
 */
void ltc6811_clraux(void)
{
  uint8_t cmd[2]= {0x07 , 0x12};
  cmd_68(cmd);
}

/**
 * The command clears the cell voltage registers and initializes all values
 * to 1. The register will read back 0xFF after the command is sent.
 */
void ltc6811_clrcell(void)
{
    uint8_t cmd[2] = { 0x07, 0x11 };
    cmd_68(cmd);
}

/*
 * The command clears the Stat registers and intiallizes
 * all values to 1. The register will read back hexadecimal 0xFF
 * after the command is sent.
 *
 */
void ltc6811_clrstat(void)
{
  uint8_t cmd[2]= {0x07 , 0x13};
  cmd_68(cmd);
}

/**
 * Starts cell voltage self test conversion
 *
 * @param ADC Mode
 * @param Self Test
 *
 */
void ltc6811_cvst(uint8_t MD, uint8_t ST)
{
  uint8_t cmd[2];
  uint8_t md_bits;

  md_bits = (MD & 0x02) >> 1;
  cmd[0] = md_bits + 0x02;
  md_bits = (MD & 0x01) << 7;
  cmd[1] =  md_bits + ((ST)<<5) +0x07;
  cmd_68(cmd);
}

/**
 * Starts the Mux Decoder diagnostic self test.
 *
 */
void ltc6811_diagn(void)
{
    uint8_t cmd[2] = { 0x07, 0x15 };
    cmd_68(cmd);
}

/**
 * Helper function to intialize CFG variables.
 *
 */
void ltc6811_init_cfg(cell_asic ic[])
{
    bool REFON = true;  // have refon always for faster adc conversion
    bool ADCOPT = false;    // select 26hz, or 7kHz mode
    bool gpioBits[5] = { true, true, true, true, true };    // set PDR off
    bool dccBits[12] = { false, false, false, false, false, false, false, false,
                         false, false, false, false };  // initialize all S switches to OFF

    uint8_t current_ic = 0;
    for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
    {
        uint32_t j = 0;
        for (j = 0; j < 6; j++)
        {
            ic[current_ic].config.tx_data[j] = 0;
            ic[current_ic].configb.tx_data[j] = 0;
        }

        ltc6811_set_cfgr(current_ic, ic, REFON, ADCOPT, gpioBits, dccBits);
    }
}

/**
 * The limits configured in this function are for the functions that process
 * data that comes from the ltc6811. They are usually bounds on loops. These
 * limits correspond to the number of channels found in the datasheet.
 */
void ltc6811_init_reg_limits(cell_asic ic[])
{
    uint32_t cic = 0;
    for (cic = 0; cic < TOTAL_ICS; cic++)
    {
        ic[cic].ic_reg.cell_channels = 12;
        ic[cic].ic_reg.stat_channels = 4;
        ic[cic].ic_reg.aux_channels = 6;
        ic[cic].ic_reg.num_cv_reg = 4;
        ic[cic].ic_reg.num_gpio_reg = 2;
        ic[cic].ic_reg.num_stat_reg = 3;
    }
}

/**
 * This function will block operation until the ADC has finished it's
 * conversion.
 */
uint32_t ltc6811_pollAdc(void)
{
    uint32_t counter = 0;
    uint8_t finished = 0;
    uint8_t current_time = 0;
    uint8_t cmd[4];
    uint16_t cmd_pec;

    cmd[0] = 0x07;
    cmd[1] = 0x14;
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_array(4, cmd, SSI1_BASE);

    while ((counter < 200000) && (finished == 0))
    {
        current_time = (uint8_t) spiTransfer(0xff, SSI1_BASE);
        if (current_time > 0)
        {
            finished = 1;
        }
        else
        {
            counter = counter + 10;
        }
    }

    setCSHigh(SSI1_BASE);

    return (counter);
}

/**
 * Helper Function to reset PEC counters
 */
void ltc6811_reset_crc_count(cell_asic ic[])
{
    uint32_t current_ic = 0;
    for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
    {
        ic[current_ic].crc_count.pec_count = 0;
        ic[current_ic].crc_count.cfgr_pec = 0;

        uint32_t i = 0;
        for (i = 0; i < 6; i++)
        {
            ic[current_ic].crc_count.cell_pec[i] = 0;

        }
        for (i = 0; i < 4; i++)
        {
            ic[current_ic].crc_count.aux_pec[i] = 0;
        }
        for (i = 0; i < 2; i++)
        {
            ic[current_ic].crc_count.stat_pec[i] = 0;
        }
    }
}

/*
 * The function is used to read the parsed GPIO codes of the ltc6811. This
 * function will send the requested read commands, parse the data, and store
 * the gpio voltages in aux_codes variable
 *
 * @param Determines which GPIO voltage register is read back..
 * @param the number of ICs in the system
 * @param A two dimensional array of the gpio voltage codes
 *
 */
int8_t ltc6811_rdaux(uint8_t reg, cell_asic ic[])
{
    uint8_t data[NUM_RX_BYT * TOTAL_ICS];
    int8_t pec_error = 0;
    uint8_t c_ic = 0;

    if (reg == 0)
    {
        // executes once for each of the ltc6811 aux voltage registers
        uint32_t gpio_reg = 1;
        for (gpio_reg = 1; gpio_reg < ic[0].ic_reg.num_gpio_reg + 1; gpio_reg++)
        {
            ltc6811_rdaux_reg(gpio_reg, data); //Reads the raw auxiliary register data into the data[] array

            uint32_t current_ic = 0;
            for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
            {
                if (ic->isospi_reverse == false)
                {
                    c_ic = current_ic;
                }
                else
                {
                    c_ic = TOTAL_ICS - current_ic - 1;
                }
                pec_error = parse_cells(current_ic, gpio_reg, data,
                                        &ic[c_ic].aux.a_codes[0],
                                        &ic[c_ic].aux.pec_match[0]);
            }
        }
    }
    else
    {
        ltc6811_rdaux_reg(reg, data);

        uint32_t current_ic = 0;
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
            if (ic->isospi_reverse == false)
            {
                c_ic = current_ic;
            }
            else
            {
                c_ic = TOTAL_ICS - current_ic - 1;
            }
            pec_error = parse_cells(current_ic, reg, data,
                                    &ic[c_ic].aux.a_codes[0],
                                    &ic[c_ic].aux.pec_match[0]);
        }
    }
    ltc6811_check_pec(AUX, ic);
    return (pec_error);
}

/*
 * The function reads a single GPIO voltage register and stores the read data
 * in the *data point as a byte array. This function is rarely used outside of
 * the ltc6811_rdaux() command.
 *
 * @param Determines which GPIO voltage register is read back
 * @param The number of ICs in the system
 * @param Array of the unparsed auxiliary codes
 */
void ltc6811_rdaux_reg(uint8_t reg, uint8_t *data)
{
    const uint8_t REG_LEN = 8; // number of bytes in the register + 2 bytes for the PEC
    uint8_t cmd[4];
    uint16_t cmd_pec;

    if (reg == 1)     //Read back auxiliary group A
    {
        cmd[1] = 0x0C;
        cmd[0] = 0x00;
    }
    else if (reg == 2)  //Read back auxiliary group B
    {
        cmd[1] = 0x0e;
        cmd[0] = 0x00;
    }
    else if (reg == 3)  //Read back auxiliary group C
    {
        cmd[1] = 0x0D;
        cmd[0] = 0x00;
    }
    else if (reg == 4)  //Read back auxiliary group D
    {
        cmd[1] = 0x0F;
        cmd[0] = 0x00;
    }
    else          //Read back auxiliary group A
    {
        cmd[1] = 0x0C;
        cmd[0] = 0x00;
    }

    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_read(cmd, 4, data, (REG_LEN * TOTAL_ICS), SSI1_BASE);
    setCSHigh(SSI1_BASE);
}

/**
 * @brief Read CFGA
 *
 * @param number of ICs in the system
 * @param
 */
int8_t ltc6811_rdcfg(cell_asic ic[])
{
    uint8_t cmd[2] = { 0x00, 0x02 };
    uint8_t read_buffer[256];
    int8_t pec_error = 0;
    uint16_t data_pec;
    uint16_t calc_pec;
    uint8_t c_ic = 0;

    pec_error = read_68(cmd, read_buffer);

    uint32_t current_ic = 0;
    for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
    {
        if (ic->isospi_reverse == false)
        {
            c_ic = current_ic;
        }
        else
        {
            c_ic = TOTAL_ICS - current_ic - 1;
        }

        uint32_t byte = 0;
        for (byte = 0; byte < 8; byte++)
        {
            ic[c_ic].config.rx_data[byte] =
                    read_buffer[byte + (8 * current_ic)];
        }
        calc_pec = pec15_calc(6, &read_buffer[8 * current_ic]);
        data_pec = read_buffer[7 + (8 * current_ic)]
                | (read_buffer[6 + (8 * current_ic)] << 8);
        if (calc_pec != data_pec)
        {
            ic[c_ic].config.rx_pec_match = 1;
        }
        else
        {
            ic[c_ic].config.rx_pec_match = 0;
        }
    }
    ltc6811_check_pec(CFGR, ic);
    return (pec_error);
}

/**
 * Reads and parses the ltc681x cell voltage registers.
 *
 * @param controls which cell voltage register is read back
 * @param array of the parsed cell codes
 */
uint8_t ltc6811_rdcv(uint8_t reg, cell_asic ic[])
{
    int8_t pec_error = 0;
    uint8_t cell_data[NUM_RX_BYT * TOTAL_ICS];
    uint8_t c_ic = 0;

    if (reg == 0)
    {
        uint32_t cell_reg = 0;
        for (cell_reg = 1; cell_reg < ic[0].ic_reg.num_cv_reg + 1; cell_reg++) //executes once for each of the ltc6811 cell voltage registers
        {
            ltc6811_rdcv_reg(cell_reg, cell_data);

            uint32_t current_ic = 0;
            for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
            {
                if (ic->isospi_reverse == false)
                {
                    c_ic = current_ic;
                }
                else
                {
                    c_ic = TOTAL_ICS - current_ic - 1;
                }
                pec_error = pec_error
                        + parse_cells(current_ic, cell_reg, cell_data,
                                      &ic[c_ic].cells.c_codes[0],
                                      &ic[c_ic].cells.pec_match[0]);
            }
        }
    }

    else
    {
        ltc6811_rdcv_reg(reg, cell_data);

        uint32_t current_ic = 0;
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
        {
            if (ic->isospi_reverse == false)
            {
                c_ic = current_ic;
            }
            else
            {
                c_ic = TOTAL_ICS - current_ic - 1;
            }
            pec_error = pec_error
                    + parse_cells(current_ic, reg, &cell_data[8 * c_ic],
                                  &ic[c_ic].cells.c_codes[0],
                                  &ic[c_ic].cells.pec_match[0]);
        }
    }
    ltc6811_check_pec(CELL, ic);
    return (pec_error);
}

/**
 * Reads the raw cell voltage register data
 *
 * @param Determines which cell voltage register is read back
 * @param the number of ICs in the
 * @param An array of the unparsed cell codes
 */
void ltc6811_rdcv_reg(uint8_t reg, uint8_t * data)
{
    const uint8_t REG_LEN = 8; //number of bytes in each ICs register + 2 bytes for the PEC
    uint8_t cmd[4];
    uint16_t cmd_pec;

    if (reg == 1)     //1: RDCVA
    {
        cmd[1] = 0x04;
        cmd[0] = 0x00;
    }
    else if (reg == 2) //2: RDCVB
    {
        cmd[1] = 0x06;
        cmd[0] = 0x00;
    }
    else if (reg == 3) //3: RDCVC
    {
        cmd[1] = 0x08;
        cmd[0] = 0x00;
    }
    else if (reg == 4) //4: RDCVD
    {
        cmd[1] = 0x0A;
        cmd[0] = 0x00;
    }
    else if (reg == 5) //4: RDCVE
    {
        cmd[1] = 0x09;
        cmd[0] = 0x00;
    }
    else if (reg == 6) //4: RDCVF
    {
        cmd[1] = 0x0B;
        cmd[0] = 0x00;
    }

    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_read(cmd, 4, data, (REG_LEN * TOTAL_ICS), SSI1_BASE);
    setCSHigh(SSI1_BASE);
}

/*
 *  Reads and parses the ltc681x stat registers.
 *
 *  @param determines which stat register is read back
 *  @param the number of ICs in the system
 *  @param
 *
 */
int8_t ltc6811_rdstat(uint8_t reg, cell_asic ic[])
{
    const uint8_t BYT_IN_REG = 6;
    const uint8_t GPIO_IN_REG = 3;

    uint8_t data[NUM_RX_BYT * TOTAL_ICS];
    uint8_t data_counter = 0;
    int8_t pec_error = 0;
    uint16_t parsed_stat;
    uint16_t received_pec;
    uint16_t data_pec;
    uint8_t c_ic = 0;

    if (reg == 0)
    {
        uint32_t stat_reg = 1;
        for (stat_reg = 1; stat_reg < 3; stat_reg++) //executes once for each of the ltc6811 stat voltage registers
        {
            data_counter = 0;
            ltc6811_rdstat_reg(stat_reg, data); //Reads the raw statiliary register data into the data[] array

            uint32_t current_ic = 0;
            for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++) // executes for every ltc6811 in the daisy chain
            {
                if (ic->isospi_reverse == false)
                {
                    c_ic = current_ic;
                }
                else
                {
                    c_ic = TOTAL_ICS - current_ic - 1;
                }
                // current_ic is used as the IC counter
                if (stat_reg == 1)
                {
                    uint32_t current_gpio = 0;
                    // This loop parses the read back data into GPIO voltages, it
                    // loops once for each of the 3 gpio voltage codes in the register
                    for (current_gpio = 0; current_gpio < GPIO_IN_REG; current_gpio++)
                    {
                        parsed_stat = data[data_counter]
                                + (data[data_counter + 1] << 8); //Each gpio codes is received as two bytes and is combined to
                        ic[c_ic].stat.stat_codes[current_gpio] = parsed_stat;
                        data_counter = data_counter + 2; //Because gpio voltage codes are two bytes the data counter

                    }
                }
                else if (stat_reg == 2)
                {
                    parsed_stat = data[data_counter]
                            + (data[data_counter + 1] << 8); //Each gpio codes is received as two bytes and is combined to
                    data_counter = data_counter + 2;
                    ic[c_ic].stat.stat_codes[3] = parsed_stat;
                    ic[c_ic].stat.flags[0] = data[data_counter++];
                    ic[c_ic].stat.flags[1] = data[data_counter++];
                    ic[c_ic].stat.flags[2] = data[data_counter++];
                    ic[c_ic].stat.mux_fail[0] = (data[data_counter] & 0x02)
                            >> 1;
                    ic[c_ic].stat.thsd[0] = data[data_counter++] & 0x01;
                }

                received_pec = (data[data_counter] << 8)
                        + data[data_counter + 1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
                //after the 6 gpio voltage data bytes
                data_pec = pec15_calc(BYT_IN_REG, &data[current_ic * NUM_RX_BYT]);

                if (received_pec != data_pec)
                {
                    pec_error = -1; //The pec_error variable is simply set negative if any PEC errors
                    ic[c_ic].stat.pec_match[stat_reg - 1] = 1;
                    //are detected in the received serial data
                }
                else
                {
                    ic[c_ic].stat.pec_match[stat_reg - 1] = 0;
                }

                data_counter = data_counter + 2; //Because the transmitted PEC code is 2 bytes long the data_counter
                //must be incremented by 2 bytes to point to the next ICs gpio voltage data
            }
        }
    }
    else
    {
        ltc6811_rdstat_reg(reg, data);

        uint32_t current_ic = 0;
        for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++) // executes for every ltc6811 in the daisy chain
        {
            // current_ic is used as an IC counter
            if (ic->isospi_reverse == false)
            {
                c_ic = current_ic;
            }
            else
            {
                c_ic = TOTAL_ICS - current_ic - 1;
            }
            if (reg == 1)
            {
                uint32_t current_gpio = 0;
                for (current_gpio = 0; current_gpio < GPIO_IN_REG; current_gpio++) // This loop parses the read back data into GPIO voltages, it
                {
                    // loops once for each of the 3 gpio voltage codes in the register
                    parsed_stat = data[data_counter]
                            + (data[data_counter + 1] << 8); //Each gpio codes is received as two bytes and is combined to
                    // create the parsed gpio voltage code

                    ic[c_ic].stat.stat_codes[current_gpio] = parsed_stat;
                    data_counter = data_counter + 2; //Because gpio voltage codes are two bytes the data counter
                    //must increment by two for each parsed gpio voltage code
                }
            }
            else if (reg == 2)
            {
                parsed_stat = data[data_counter++]
                        + (data[data_counter++] << 8); //Each gpio codes is received as two bytes and is combined to
                ic[c_ic].stat.stat_codes[3] = parsed_stat;
                ic[c_ic].stat.flags[0] = data[data_counter++];
                ic[c_ic].stat.flags[1] = data[data_counter++];
                ic[c_ic].stat.flags[2] = data[data_counter++];
                ic[c_ic].stat.mux_fail[0] = (data[data_counter] & 0x02) >> 1;
                ic[c_ic].stat.thsd[0] = data[data_counter++] & 0x01;
            }

            received_pec = (data[data_counter] << 8) + data[data_counter + 1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
            //after the 6 gpio voltage data bytes
            data_pec = pec15_calc(BYT_IN_REG, &data[current_ic * NUM_RX_BYT]);
            if (received_pec != data_pec)
            {
                pec_error = -1; //The pec_error variable is simply set negative if any PEC errors
                ic[c_ic].stat.pec_match[reg - 1] = 1;
            }

            data_counter = data_counter + 2;
        }
    }
    ltc6811_check_pec(STAT, ic);
    return (pec_error);
}

/**
 * The function reads a single stat  register and stores the read data
 * in the *data point as a byte array. This function is rarely used outside of
 * the ltc6811_rdstat() command.
 *
 * @param determines which stat register is read back
 * @param the number of ICs in the system
 * @param array of the unparsed stat codes
 *
 */
void ltc6811_rdstat_reg(uint8_t reg, uint8_t *data)
{
    const uint8_t REG_LEN = 8; // number of bytes in the register + 2 bytes for the PEC
    uint8_t cmd[4];
    uint16_t cmd_pec;

    if (reg == 1)     //Read back statiliary group A
    {
        cmd[1] = 0x10;
        cmd[0] = 0x00;
    }
    else if (reg == 2)  //Read back statiliary group B
    {
        cmd[1] = 0x12;
        cmd[0] = 0x00;
    }
    else          //Read back statiliary group A
    {
        cmd[1] = 0x10;
        cmd[0] = 0x00;
    }

    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t) (cmd_pec >> 8);
    cmd[3] = (uint8_t) (cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_read(cmd, 4, data, (REG_LEN * TOTAL_ICS), SSI1_BASE);
    setCSHigh(SSI1_BASE);
}

/**
 * Runs the Digital Filter Self Test
 */
int16_t ltc6811_run_cell_adc_st(uint8_t adc_reg, cell_asic ic[])
{
    int16_t error = 0;
    uint16_t expected_result = 0;

    uint32_t self_test = 1;
    for (self_test = 1; self_test < 3; self_test++)
    {
        uint32_t cic = 0; // loop variable for for loops in switch statement

        expected_result = ltc6811_st_lookup(2, self_test);
        wakeup_idle();
        switch (adc_reg)
        {
        case CELL:
            wakeup_idle();
            ltc6811_clrcell();
            ltc6811_cvst(2, self_test);
            ltc6811_pollAdc(); //this isn't working
            wakeup_idle();
            error = ltc6811_rdcv(0, ic);


            for (cic = 0; cic < TOTAL_ICS; cic++)
            {
                uint32_t channel = 0;
                for (channel = 0; channel < ic[cic].ic_reg.cell_channels; channel++)
                {
                    if (ic[cic].cells.c_codes[channel] != expected_result)
                    {
                        error = error + 1;
                    }
                }
            }
            break;
        case AUX:
            error = 0;
            wakeup_idle();
            ltc6811_clraux();
            ltc6811_axst(2, self_test);
            ltc6811_pollAdc();
            SysCtlDelay(800000); // delay 10 millisecons
            wakeup_idle();
            ltc6811_rdaux(0, ic);

            for (cic = 0; cic < TOTAL_ICS; cic++)
            {
                uint32_t channel = 0;
                for (channel = 0; channel < ic[cic].ic_reg.aux_channels; channel++)
                {
                    if (ic[cic].aux.a_codes[channel] != expected_result)
                    {
                        error = error + 1;
                    }
                }
            }
            break;
        case STAT:
            wakeup_idle();
            ltc6811_clrstat();
            ltc6811_statst(2, self_test);
            ltc6811_pollAdc();
            wakeup_idle();
            error = ltc6811_rdstat(0, ic);

            for (cic = 0; cic < TOTAL_ICS; cic++)
            {
                uint32_t channel = 0;
                for (channel = 0; channel < ic[cic].ic_reg.stat_channels; channel++)
                {
                    if (ic[cic].stat.stat_codes[channel] != expected_result)
                    {
                        error = error + 1;
                    }
                }
            }
            break;

        default:
            error = -1;
            break;
        }
    }
    return (error);
}

/**
 * Runs the ADC overlap test for the IC
 */

uint16_t ltc6811_run_adc_overlap(cell_asic ic[])
{
    uint16_t error = 0;
    int32_t measure_delta = 0;
    int16_t failure_pos_limit = 20;
    int16_t failure_neg_limit = -20;

    wakeup_idle();
    ltc6811_adol(MD_7KHZ_3KHZ, DCP_DISABLED);
    ltc6811_pollAdc();
    wakeup_idle();
    error = ltc6811_rdcv(0, ic);

    uint32_t cic = 0;
    for (cic = 0; cic < TOTAL_ICS; cic++)
    {
        measure_delta = (int32_t) ic[cic].cells.c_codes[6]
                - (int32_t) ic[cic].cells.c_codes[7];
        if ((measure_delta > failure_pos_limit)
                || (measure_delta < failure_neg_limit))
        {
            error = error | (1 << (cic - 1));
        }
    }
    return (error);
}

/**
 * @brief Runs the redundancy self test.
 *
 */
int16_t ltc6811_run_adc_redundancy_st(uint8_t adc_mode, uint8_t adc_reg,
                                      cell_asic ic[])
{
    int16_t error = 0;

    uint32_t cic = 0; // loop variable for for loops inside switch statement
    uint32_t self_test = 1;
    for (self_test = 1; self_test < 3; self_test++)
    {
        wakeup_idle();
        switch (adc_reg)
        {
        case AUX:
            ltc6811_clraux();
            ltc6811_adaxd(adc_mode, AUX_CH_ALL);
            ltc6811_pollAdc();
            wakeup_idle();
            error = ltc6811_rdaux(0, ic);


            for (cic = 0; cic < TOTAL_ICS; cic++)
            {
                uint32_t channel = 0;
                for (channel = 0; channel < ic[cic].ic_reg.aux_channels; channel++)
                {
                    if (ic[cic].aux.a_codes[channel] >= 65280)
                    {
                        error = error + 1;
                    }
                }
            }
            break;

        case STAT:
            ltc6811_clrstat();
            ltc6811_adstatd(adc_mode, STAT_CH_ALL);
            ltc6811_pollAdc();
            wakeup_idle();
            error = ltc6811_rdstat(0, ic);
            for (cic = 0; cic < TOTAL_ICS; cic++)
            {
                uint32_t channel = 0;
                for (channel = 0; channel < ic[cic].ic_reg.stat_channels; channel++)
                {
                    if (ic[cic].stat.stat_codes[channel] >= 65280)
                    {
                        error = error + 1;
                    }
                }
            }
            break;

        default:
            error = -1;
            break;
        }
    }

    return (error);
}

/**
 * Runs the datasheet algorithm for open wire
 *
 */
uint8_t ltc6811_run_openwire(cell_asic ic[])
{
    uint16_t OPENWIRE_THRESHOLD = 4000;
    const uint8_t N_CHANNELS = ic[0].ic_reg.cell_channels;

    cell_asic pullUp_cell_codes[TOTAL_ICS];
    cell_asic pullDwn_cell_codes[TOTAL_ICS];
    cell_asic openWire_delta[TOTAL_ICS];
    uint8_t error;

    // setup array size / limit variables
    ltc6811_init_reg_limits(pullUp_cell_codes);
    ltc6811_init_reg_limits(pullDwn_cell_codes);
    ltc6811_init_reg_limits(openWire_delta);

    wakeup_sleep();
    ltc6811_adow(MD_7KHZ_3KHZ, PULL_UP_CURRENT);
    ltc6811_pollAdc();
    wakeup_idle();
    ltc6811_adow(MD_7KHZ_3KHZ, PULL_UP_CURRENT);
    ltc6811_pollAdc();
    wakeup_idle();
    error = ltc6811_rdcv(0, pullUp_cell_codes);

    wakeup_idle();
    ltc6811_adow(MD_7KHZ_3KHZ, PULL_DOWN_CURRENT);
    ltc6811_pollAdc();
    wakeup_idle();
    ltc6811_adow(MD_7KHZ_3KHZ, PULL_DOWN_CURRENT);
    ltc6811_pollAdc();
    wakeup_idle();
    error = ltc6811_rdcv(0, pullDwn_cell_codes);

    uint32_t cic = 0;
    for (cic = 0; cic < TOTAL_ICS; cic++)
    {
        ic[cic].system_open_wire = 0;

        uint32_t cell = 0;
        for (cell = 0; cell < N_CHANNELS; cell++)
        {
            if (pullDwn_cell_codes[cic].cells.c_codes[cell]
                    > pullUp_cell_codes[cic].cells.c_codes[cell])
            {
                openWire_delta[cic].cells.c_codes[cell] =
                        pullDwn_cell_codes[cic].cells.c_codes[cell]
                                - pullUp_cell_codes[cic].cells.c_codes[cell];
            }
            else
            {
                openWire_delta[cic].cells.c_codes[cell] = 0;
            }
        }
    }

    for (cic = 0; cic < TOTAL_ICS; cic++)
    {
        uint32_t cell = 0;
        for (cell = 1; cell < N_CHANNELS; cell++)
        {
            if (openWire_delta[cic].cells.c_codes[cell] > OPENWIRE_THRESHOLD)
            {
                ic[cic].system_open_wire += (1 << cell);
            }
        }

        if (pullUp_cell_codes[cic].cells.c_codes[0] == 0)
        {
            ic[cic].system_open_wire += 1;
        }

        if (pullUp_cell_codes[cic].cells.c_codes[N_CHANNELS - 1] == 0)
        {
            ic[cic].system_open_wire += (1 << (N_CHANNELS));
        }
    }

    return error;
}

/**
 * Helper function to set CFGR variable
 *
 */
void ltc6811_set_cfgr(uint8_t nIC, cell_asic ic[], bool refon, bool adcopt,
                      bool gpio[5], bool dcc[12])
{
    ltc6811_set_cfgr_refon(nIC, ic, refon);
    ltc6811_set_cfgr_adcopt(nIC, ic, adcopt);
    ltc6811_set_cfgr_gpio(nIC, ic, gpio);
    ltc6811_set_cfgr_dis(nIC, ic, dcc);
}

/**
 * Helper function to set the adcopt bit
 */
void ltc6811_set_cfgr_adcopt(uint8_t nIC, cell_asic ic[], bool adcopt)
{
    if (adcopt)
    {
        ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0] | 0x01;
    }
    else
    {
        ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0] & 0xFE;
    }
}

/*
 * Helper function to control discharge
 */
void ltc6811_set_cfgr_dis(uint8_t nIC, cell_asic ic[], bool dcc[12])
{
    uint32_t i = 0;
    for (i = 0; i < 8; i++)
    {
        if (dcc[i])
        {
            ic[nIC].config.tx_data[4] = ic[nIC].config.tx_data[4] | (0x01 << i);
        }
        else
        {
            ic[nIC].config.tx_data[4] = ic[nIC].config.tx_data[4]
                    & (~(0x01 << i));
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (dcc[i + 8])
        {
            ic[nIC].config.tx_data[5] = ic[nIC].config.tx_data[5] | (0x01 << i);
        }
        else
        {
            ic[nIC].config.tx_data[5] = ic[nIC].config.tx_data[5]
                    & (~(0x01 << i));
        }
    }
}

/*
 * Helper function to set GPIO bits
 */
void ltc6811_set_cfgr_gpio(uint8_t nIC, cell_asic ic[], bool gpio[5])
{
    uint32_t i = 0;
    for (i = 0; i < 5; i++)
    {
        if (gpio[i])
        {
            ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]
                    | (0x01 << (i + 3));
        }
        else
        {
            ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]
                    & (~(0x01 << (i + 3)));
        }
    }
}

/*
 * helper function to set OV value in CFG register
 */
void ltc6811_set_cfgr_ov(uint8_t nIC, cell_asic ic[], uint16_t ov)
{
    uint16_t tmp = (ov / 16);
    ic[nIC].config.tx_data[3] = 0x00FF & (tmp >> 4);
    ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2] & 0x0F;
    ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2]
            | ((0x000F & tmp) << 4);
}

/*
 * Helper function to set the REFON bit
 */
void ltc6811_set_cfgr_refon(uint8_t nIC, cell_asic ic[], bool refon)
{
    if (refon)
    {
        ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0] | 0x04;
    }
    else
    {
        ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0] & 0xFB;
    }
}

/*
 * Helper Function to set uv value in CFG register
 *
 */
void ltc6811_set_cfgr_uv(uint8_t nIC, cell_asic ic[], uint16_t uv)
{
    uint16_t tmp = (uv / 16) - 1;
    ic[nIC].config.tx_data[1] = 0x00FF & tmp;
    ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2] & 0xF0;
    ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2]
            | ((0x0F00 & tmp) >> 8);
}

/**
 * Helper function to set discharge bit in CFG register. After setting the
 * discharge bit, the ltc6811_wrcfg function must be called.
 *
 */
void ltc6811_set_discharge(int Cell, cell_asic ic[])
{
    uint32_t i = 0;
    for (i = 0; i < TOTAL_ICS; i++)
    {
        if (Cell < 9)
        {
            ic[i].config.tx_data[4] = ic[i].config.tx_data[4] | (1 << (Cell - 1));
        }
        else if (Cell < 13)
        {
            ic[i].config.tx_data[5] = ic[i].config.tx_data[5] | (1 << (Cell - 9));
        }
    }
}

/**
 * Start a Status Register Self Test Conversion
 *
 */
void ltc6811_statst(uint8_t MD, uint8_t ST)
{
    uint8_t cmd[2];
    uint8_t md_bits;

    md_bits = (MD & 0x02) >> 1;
    cmd[0] = md_bits + 0x04;
    md_bits = (MD & 0x01) << 7;
    cmd[1] = md_bits + ((ST & 0x03) << 5) + 0x0F;
    cmd_68(cmd);
}

/**
 * Looks up the result pattern for digital filter self test
 *
 * @param ADC Mode
 * @param Self Test
 *
 */
uint16_t ltc6811_st_lookup(uint8_t MD, uint8_t ST)
{
    uint16_t test_pattern = 0;
    if (MD == 1)
    {
        if (ST == 1)
        {
            test_pattern = 0x9565;
        }
        else
        {
            test_pattern = 0x6A9A;
        }
    }
    else
    {
        if (ST == 1)
        {
            test_pattern = 0x9555;
        }
        else
        {
            test_pattern = 0x6AAA;
        }
    }
    return (test_pattern);
}

/**
 * Write the ltc681x CFGRA
 *
 * @param The number of ICs being written to
 *
 */
void ltc6811_wrcfg(cell_asic ic[])
{
    uint8_t cmd[2] = { 0x00, 0x01 };
    uint8_t write_buffer[256];
    uint8_t write_count = 0;
    uint8_t c_ic = 0;

    uint8_t current_ic = 0;
    for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
    {
        if (ic->isospi_reverse == false)
        {
            c_ic = current_ic;
        }
        else
        {
            c_ic = TOTAL_ICS - current_ic - 1;
        }

        uint8_t data = 0;
        for (data = 0; data < 6; data++)
        {
            write_buffer[write_count] = ic[c_ic].config.tx_data[data];
            write_count++;
        }
    }
    write_68(cmd, write_buffer);
}

/* Shifts data in COMM register out over LTC681x SPI/I2C port */
void ltc6811_stcomm(uint8_t len) //Length of data to be transmitted
{
    uint8_t cmd[4];
    uint16_t cmd_pec;

    cmd[0] = 0x07;
    cmd[1] = 0x23;
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    setCSLow(SSI1_BASE);
    spi_write_array(4, cmd, SSI1_BASE);
    int i = 0;
    for (i = 0; i<len*3; i++)
    {
        spiTransfer(0xff, SSI1_BASE);
    }
    
    setCSHigh(SSI1_BASE);
}

/* Writes the comm register */
void ltc6811_wrcomm()
{
    uint8_t cmd[2] = {0x07, 0x21};
    uint8_t write_buffer[256];
    uint8_t write_count = 0;
    uint8_t c_ic = 0;

    uint8_t current_ic = 0;
    for (current_ic = 0; current_ic < TOTAL_ICS; current_ic++)
    {
        if (bms_ic->isospi_reverse == false)
        {
            c_ic = current_ic;
        }
        else
        {
            c_ic = TOTAL_ICS - current_ic - 1;
        }

        uint8_t data = 0;
        for (data = 0; data < 6; data++)
        {
            write_buffer[write_count] = bms_ic[c_ic].com.tx_data[data];
            write_count++;
        }
    }
    write_68(cmd, write_buffer);
}
/* Reads COMM registers of a LTC681x daisy chain */
int8_t ltc6811_rdcomm(cell_asic ic[])
{
	uint8_t cmd[2]= {0x07 , 0x22};
	uint8_t read_buffer[256];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t calc_pec;
	uint8_t c_ic=0;
	
	pec_error = read_68(cmd, read_buffer);
	uint8_t current_ic;
	for (current_ic = 0; current_ic<TOTAL_ICS; current_ic++)
	{
		if (ic->isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = TOTAL_ICS - current_ic - 1;
		}
	
        uint8_t byte;
		for (byte=0; byte<8; byte++)
		{
			ic[c_ic].com.rx_data[byte] = read_buffer[byte+(8*current_ic)];
		}
		
		calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
		data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
		if (calc_pec != data_pec )
		{
			ic[c_ic].com.rx_pec_match = 1;
		}
		else ic[c_ic].com.rx_pec_match = 0;
	}
	
    return(pec_error);
}

//TODO: delete this
void print_wrcomm()
{
    uint16_t comm_pec;
    uint8_t t;
    uint8_t l;
    uint8_t transmitted[6];

    uint8_t i;
    for (i = 0; i < 6; i++)
    {
        transmitted[i] = bms_ic[0].com.tx_data[i];
    }

    comm_pec = pec15_calc(6,&bms_ic[0].com.tx_data[0]);
    t = (uint8_t)(comm_pec>>8);
    l = (uint8_t)(comm_pec);
}

/* Load tx_data into bms struct */
void ltc6811_set_tx_data(uint8_t ic_i, uint8_t data[6])
{
    uint8_t i = 0;
    for (i = 0; i < 6; i++)
    {
        bms_ic[ic_i].com.tx_data[i] = data[i];
        // bms_ic[ic_i].com.tx_data[i] = i;
    }
}

// /*
// The function is used to read the  parsed GPIO codes of the LTC681x.
// This function will send the requested read commands parse the data
// and store the gpio voltages in a_codes variable.
// */
// int8_t ltc6811_rdaux(uint8_t reg, cell_asic *ic)
// {
//     uint8_t *data;
//     int8_t pec_error = 0;
//     uint8_t c_ic =0;
//     data = (uint8_t *) malloc((NUM_RX_BYT*TOTAL_ICS)*sizeof(uint8_t));

//     if (reg == 0)
//     {
//         for (uint8_t gpio_reg = 1; gpio_reg<ic[0].ic_reg.num_gpio_reg+1; gpio_reg++) //Executes once for each of the LTC681x aux voltage registers
//         {
//             ltc6811_rdaux_reg(gpio_reg, data);                 //Reads the raw auxiliary register data into the data[] array
//             for (int current_ic = 0; current_ic<TOTAL_ICS; current_ic++)
//             {
//                 if (ic->isospi_reverse == false)
//                 {
//                   c_ic = current_ic;
//                 }
//                 else
//                 {
//                   c_ic = TOTAL_ICS - current_ic - 1;
//                 }
//                 pec_error = parse_cells(current_ic,gpio_reg, data,
//                                         &ic[c_ic].aux.a_codes[0],
//                                         &ic[c_ic].aux.pec_match[0]);
//             }
//         }
//     }
//     else
//     {
//         ltc6811_rdaux_reg(reg, data);

//         for (int current_ic = 0; current_ic<TOTAL_ICS; current_ic++)
//         {
//             if (ic->isospi_reverse == false)
//             {
//                 c_ic = current_ic;
//             }
//             else
//             {
//                 c_ic = TOTAL_ICS - current_ic - 1;
//             }
//             pec_error = parse_cells(current_ic,reg, data,
//                                   &ic[c_ic].aux.a_codes[0],
//                                   &ic[c_ic].aux.pec_match[0]);
//         }
//     }

//     //TODO: this needs to change v
//     LTC681x_check_pec(total_ic,AUX,ic);
//     free(data);

//     return (pec_error);
// }

// /*
// The function reads a single GPIO voltage register and stores the read data
// in the *data point as a byte array. This function is rarely used outside of
// the ltc6811_rdaux() command.
// */
// void ltc6811_rdaux_reg(uint8_t reg, //Determines which GPIO voltage register is read back
//                        uint8_t *data //Array of the unparsed auxiliary codes
//                       )
// {
//     const uint8_t REG_LEN = 8; // Number of bytes in the register + 2 bytes for the PEC
//     uint8_t cmd[4];
//     uint16_t cmd_pec;

//     if (reg == 1)     //Read back auxiliary group A
//     {
//         cmd[1] = 0x0C;
//         cmd[0] = 0x00;
//     }
//     else if (reg == 2)  //Read back auxiliary group B
//     {
//         cmd[1] = 0x0E;
//         cmd[0] = 0x00;
//     }
//     else if (reg == 3)  //Read back auxiliary group C
//     {
//         cmd[1] = 0x0D;
//         cmd[0] = 0x00;
//     }
//     else if (reg == 4)  //Read back auxiliary group D
//     {
//         cmd[1] = 0x0F;
//         cmd[0] = 0x00;
//     }
//     else          //Read back auxiliary group A
//     {
//         cmd[1] = 0x0C;
//         cmd[0] = 0x00;
//     }

//     cmd_pec = pec15_calc(2, cmd);
//     cmd[2] = (uint8_t)(cmd_pec >> 8);
//     cmd[3] = (uint8_t)(cmd_pec);

//     cs_low(CS_PIN);
//     spi_write_read(cmd,4,data,(REG_LEN*TOTAL_ICS));
//     cs_high(CS_PIN);
// }
