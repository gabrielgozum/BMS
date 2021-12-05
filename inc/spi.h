/**
 *
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include <stdbool.h>
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

// ----------------------------------------------------------------------------
//
// Datatypes
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------
void SSI0_init(void);
void SSI1_init(void);
void spi_write_read(uint8_t tx_data[], uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len, uint32_t base);
void spi_write_array(uint8_t len, uint8_t data[], uint32_t base);
uint32_t spiTransfer(uint8_t tx, uint32_t base);


inline void setCSLow(uint32_t base)
{
    switch (base)
    {
    case SSI0_BASE:
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0x00);
        break;
    case SSI1_BASE:
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x00);
        break;
    default:
        break;
    }
}

inline void setCSHigh(uint32_t base)
{
    switch (base)
    {
    case SSI0_BASE:
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0xFF);
        break;
    case SSI1_BASE:
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0xFF);
        break;
    default:
        break;
    }
}

#endif /* INC_SPI_H_ */
