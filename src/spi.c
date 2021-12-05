/**
 * @brief SSI Interface
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "spi.h"


/**
 * Initialization SSI0 as SPI to communicate with the ADC IC.
 */
void SSI0_init(void)
{
    uint32_t pui32DataRx;

    // enable clocking and wait for it to be ready
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)) {};

    // GPIO function enable
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3); // CS manually controlled
    setCSHigh(SSI0_BASE); // manually set CS = 1

    // GPIO function select
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

    // configure SSI as SPI
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,
                       SSI_MODE_MASTER, 1000000, 8);

    // enable peripheral
    SSIEnable(SSI0_BASE);

    // Read any residual data from the SSI port
    while (SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx)) {};
}

/**
 * Initialization SSI1 as SPI to communicate with the BMS IC.
 */
void SSI1_init(void)
{
    uint32_t pui32DataRx;

    // enable clocking and wait for it to be ready
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI1)) {};

    // GPIO function enable
    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1); // CS manually controlled
    setCSHigh(SSI1_BASE); // manually set CS = 1

    // GPIO function select
    GPIOPinConfigure(GPIO_PD0_SSI1CLK);
    // GPIOPinConfigure(GPIO_PA3_SSI0FSS); - going to manually control CS
    GPIOPinConfigure(GPIO_PD2_SSI1RX);
    GPIOPinConfigure(GPIO_PD3_SSI1TX);

    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

    // configure SSI as SPI
    // SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,
    //                    SSI_MODE_MASTER, 1000000, 8);//1000000
    SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,
                       SSI_MODE_MASTER, 200000, 8); //Adjusted SPI clock to 200kHz

    // enable peripheral
    SSIEnable(SSI1_BASE);

    // Read any residual data from the SSI port.
    while (SSIDataGetNonBlocking(SSI1_BASE, &pui32DataRx)) {};
}

/**
 * Transfer a byte on the specified SPI Base.
 *
 * @param tx Byte to send
 * @param base SSI Base to send byte on
 *
 * @return received byte
 */
uint32_t spiTransfer(uint8_t tx, uint32_t base)
{
    uint32_t retVal;
    SSIDataPut(base, tx);
    SSIDataGet(base, &retVal);
    return retVal;
}

/*
 * Writes an array of bytes out of the SPI port
 *
 * @param len Option: Number of bytes to be written on the SPI port
 * @param data Array of bytes to be written on the SPI port
 * @param base SPI base to use
 */
void spi_write_array(uint8_t len, uint8_t data[], uint32_t base)
{
    uint32_t i = 0;

    for (i = 0; i < len; i++)
    {
        spiTransfer(data[i], base);
    }
}

/**
 * Writes and read a set number of bytes using the SPI port.
 *
 * @param array of data to be written on SPI port
 * @param length of the tx data arry
 * @param Input: array that will store the data read by the SPI port
 * @param Option: number of bytes to be read from the SPI port
 * @param base SPI base to use
 */
void spi_write_read(uint8_t tx_data[], uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len, uint32_t base)
{
    uint32_t i;

    // send data
    // will always send 4 bytes minimum - 2 for command and 2 for pec
    for (i = 0; i < tx_len; i++)
    {
        spiTransfer(tx_data[i], base);
    }

    for (i = 0; i < rx_len; i++)
    {
      rx_data[i] = (uint8_t)spiTransfer(0xff, base);
    }
}
