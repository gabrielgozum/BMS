/**
 * @brief BMS Temperatures
 *
 * This file includes functions to process temperature data
 */

#ifndef INC_TEMPERATURE_H_
#define INC_TEMPERATURE_H_

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
#define TEMP_ALPHA 0.9
#define TEMP_BETA  0.1

// ----------------------------------------------------------------------------
// Declaration
// ----------------------------------------------------------------------------


/**
 * @brief Calculate cell temperature statistics.
 *
 * Process the data received from the LTC6811 for temperatures. Calculates the
 * maximum, minimum, and average temperatures. If a sensor is faulted it
 * excludes it from the calculations.
 *
 * <REQ_44011>
 */
void temperature_stats_ct(void);

/**
 * @brief Calculates internal temperature from adc value
 *
 * Runs OOR and high/low checks and dtc handling.
 *
 * @param adc_val The adc value of the internal temperature sensor.
 */
void temperature_stats_internal(uint16_t adc_val);

/**
 * @brief Calculate cell temperature from adc value.
 *
 * Checks if the voltage read from the adc is in range before doing the
 * temperature calculation. If the voltage is OOR, it sets a fault and sets
 * the temperature to the limits of int8_t. If the temperature is valid it
 * then calculates the value. The function then checks if the temperature is
 * within the fault limits, and sets a fault if it isn't.
 *
 * @param cell_id cell to calculate the temperature for
 *
 * @returns temperature (1 LSB = 1 deg C) if in range, -128 if value is OORL,
 *          or 127 if value is OORH
 */
int8_t temperature_calc_from_adc(uint8_t cell_id);


/**
 * @brief Calculate cell temperature from gpio adc value.
 *
 * Checks if the voltage read from the adc is in range before doing the
 * temperature calculation. If the voltage is OOR, it sets a fault and sets
 * the temperature to the limits of int8_t. If the temperature is valid it
 * then calculates the value. The function then checks if the temperature is
 * within the fault limits, and sets a fault if it isn't.
 *
 * @param cell_id cell to calculate the temperature for
 *
 * @returns temperature (1 LSB = 1 deg C) if in range, -128 if value is OORL,
 *          or 127 if value is OORH
 */
int8_t temperature_calc_from_aux(uint8_t cell_id);

#endif /* INC_TEMPERATURE_H_ */
