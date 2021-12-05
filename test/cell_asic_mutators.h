/**
 * @brief Mutators for the BMS IC struct.
 *
 * @par These functions can be used to simulate data being recieved from the
 * LTC6811 and put into the BMS IC struct.
 *
 * @par This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#ifndef INC_CELL_ASIC_MUTATORS_H_
#define INC_CELL_ASIC_MUTATORS_H_

#include "ltc6811.h"

#define TEST_CELL_0_VOLTAGE_OFFSET -1420
#define TEST_CELL_9_VOLTAGE_OFFSET -1260

void cam_set_cell_voltage(cell_asic *c, uint16_t cell_voltage, uint8_t cell_num);
void cam_set_all_cell_voltages(cell_asic *c, uint16_t cell_voltage);
void cam_set_cell_temperature(cell_asic c[], uint16_t cell_voltage, uint8_t cell_num);
void cam_set_all_cell_temperatures(cell_asic *c, uint16_t cell_voltage);
void cam_set_bsbt_ratio(cell_asic c[], uint16_t ratio, uint8_t bsb_id);
void cam_set_all_bsbt_ratios(cell_asic c[], uint16_t ratio);
void cam_set_bsbh_ratio(cell_asic c[], uint16_t ratio, uint8_t bsb_id);
void cam_set_all_bsbh_ratios(cell_asic c[], uint16_t ratio);


#endif /* INC_CELL_ASIC_MUTATORS_H_ */
