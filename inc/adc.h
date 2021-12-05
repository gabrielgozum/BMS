/**
 * @brief ADC Interface
 *
 * This file includes functions to initialize the ADC peripheral, analog
 * inputs, and the external ADC.
 *
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

void adc_adc_init(void);
void adc_ain_init(void);
inline void adc_start_measurement(void);
void adc_get_measurements(uint16_t * current, uint16_t * mcu_temp);

#endif /* INC_ADC_H_ */
