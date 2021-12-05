/**
 * @brief main
 *
 * This file is the top level file for this project.  It executes all
 * initialization functions and provides the main loop for the program
 *
 */

#ifndef CODE_BMS_MAIN_H_
#define CODE_BMS_MAIN_H_

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------
void main_idle_tasks_lp(void);
void main_idle_tasks_hp(void);
void main_update_eeprom(void);
void main_update_measurements_lp(void);
void main_cell_balancing(void);
void main_check_limits_hp(void);
void main_run_openwire(void);

/**
 * Take all voltage measurements at once. The measurement is first read back
 * into the cell_asic struct and then transfered to the bms_t strcut to be
 * used to calculate stats and limits.
 */
void main_measure_voltage_all(void);

/**
 * Start the voltage measurement
 */
void main_start_measure_voltage_all(void);

/**
 * Read the voltages from the ltc6811
 */
void main_read_measure_voltage_all(void);

/**
 * Take voltage measurements off all cells. The measurement is first read back
 * into the cell_asic struct and then transfered to the bms_t struct to be
 * used to calculate statistics and limits.
 *
 * This function can be split so it only does one channel at a time.
 * Optionally somehow add in processing of the cell model after each channel is
 * read.
 */
void main_start_measure_temperature_channel(void);

void main_read_temperature_channel(void);

/**
 * Take cell temperature measurements. Only take measurements from one channel
 * group each time this function is called, and from one or two cells in the
 * channel group. The discharge switch must be enabled on the ic that reads
 * the channel for the cells to be read. DO NOT ENABLE DISCHARGE SWITCHES ON
 * ADJACENT CELLS as it creates a short circuit condition.
 */
void main_measure_temperature_channel(void);

#endif /* CODE_BMS_MAIN_H_ */
