/**
 * @brief BMS Voltages
 *
 * This file includes functions to process temperature data for cell
 * voltages.
 */

#ifndef INC_VOLTAGE_H_
#define INC_VOLTAGE_H_

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Definitions
//
// ----------------------------------------------------------------------------

/**
 * @brief Lookup Open Cell Voltage from SOC
 *
 * @param soc State of Charge
 * @return Open Cell Voltage
 */
uint16_t voltage_lookup_soc(float soc);

#endif /* INC_VOLTAGE_H_ */
