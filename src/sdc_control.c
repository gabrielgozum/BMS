/**
 * @brief SDC Interface
 *
 * This file includes functions to control the BMS's powerstage in the SDC.
 */

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "bms.h"
#include "charger.h"
#include "fault_manager.h"
#include "sdc_control.h"


void sdc_control_init(void)
{
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);

    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00);
}


void sdc_control_open(void)
{
    fault_set_aPowerStageOpen();

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00);
}


void sdc_control_close(void)
{
    fault_clear_aPowerStageOpen();

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0xFF);
}
