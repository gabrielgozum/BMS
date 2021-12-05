/*
 * ltc1380.h
 *
 *  Created on: Jun 16, 2020
 *      Author: Gabriel Gozum
 */

#ifndef INC_LTC1380_H_
#define INC_LTC1380_H_

#include <stdbool.h>
#include <stdint.h>

/*
* I2C message codes for 6811
*/
#define LTC1380_START 0b0110
#define LTC1380_BLANK 0b0000
#define LTC1380_ACK 0b0000
#define LTC1380_NACK 0b1000
#define LTC1380_NACKSTOP 0b1001
#define LTC1380_NOTRANSMIT 0b0111
#define LTC1380_STOP 0b0001

/**
 * Read channel of LTC1380 mux
 *
 * @param channel of temperature measurement
 * @param disabled disable mux
 * @return formatted I2C message for interacting with 1380 mux
 */
uint16_t ltc1380_set_channel(uint8_t channel, bool disabled);

/**
 * Load data into bms struct and transfer over I2C
 * @param transmit message to transmit
 * @return none
 */
void ltc1380_ltc6811_transmit_i2c(uint8_t* transmit);

/**
 * Format transmit[] message and call ltc1380_ltc6811_transmit_i2c()
 * @param channel of temperature measurement
 * @param disabled disable mux
 * @return none
 */
void ltc1380_format_transmit_i2c(uint8_t channel, bool Disabled);

/**
 * Top level function to control 1380 I2C mux temperature readings
 *
 * @param channel of temperature measurement
 * @return none
 */
void ltc1380_send_i2c(uint8_t channel);

#endif /* INC_LTC1380_H_ */
