/**
 * @brief Interface to the ltc1684
 *
 * This file includes functions to initialize and interface with the
 * ltc1684. This ADC reads the main HV current sensor.
 *
 * Separate functions are used to start the conversion and get the result
 * so the conversion start time can be synchronized with taking cell voltage
 * measurements.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "driverlib/ssi.h"
#include "inc/hw_memmap.h"
#include "ltc1864.h"
#include "spi.h"


void ltc1864_init(void)
{
    SSI0_init();
}


void ltc1864_start_measurement(void)
{
    setCSHigh(SSI0_BASE);
}


float ltc1864_get_measurement(void)
{
    // Enable data transfer.
    setCSLow(SSI0_BASE);

    // Read the data. Sent in two 8 bit packets.
    uint32_t data = 0;
    data = spiTransfer(0xFF, SSI0_BASE);
    data = data << 8;
    data |= spiTransfer(0xFF, SSI0_BASE);

    // Convert to voltage (read value * ref voltage / resolution).
    return data * 5.0 / 65535.0;
}
