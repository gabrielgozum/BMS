/**
 * @brief Interface to the ltc1684
 *
 * This file includes functions to initialize and interface with the
 * ltc1684. This ADC reads the main HV current sensor.
 *
 */

#ifndef INC_LTC1684_H_
#define INC_LTC1684_H_

#include <stdbool.h>

/**
 * @brief      Initialize ltc1864 hardware
 */
void ltc1864_init(void);

/**
 * @brief      Start ADC Measurement
 */
void ltc1864_start_measurement(void);

/**
 * @brief      Read ADC
 * 
 * Read the ADC and convert the ADC measurement to voltage. This function must 
 * be called at least 3.3 us after the measurement is started. 
 * (~150 clock cycles)
 *
 * Sets the CONV pin of the ltc1864 low to enable transmitting. Then, read the
 * data over the SSI connection.
 *
 * @return     measurement voltage [Volts]
 */
float ltc1864_get_measurement(void);

#endif /* INC_LTC1684_H_ */
