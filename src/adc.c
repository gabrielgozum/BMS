/**
 * @brief ADC Interface
 *
 * This file includes functions to initialize the ADC peripheral, analog
 * inputs, and the external ADC.
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_adc.h"
#include "inc/hw_memmap.h"
#include "adc.h"

/**
 * Initialize the ADC and sample sequencer to read the analog inputs
 */
void adc_adc_init(void)
{
    // Enable clocking for the ADC0 peripheral and wait for it to be ready
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while( (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) ) {};

    // configure adc0ss0 to be triggered by the mcu
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

    // configure sequence
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_TS); // mcu temp sensor
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_END | ADC_CTL_IE | ADC_CTL_CH3); // current sensor

    // enable sequencer
    ADCSequenceEnable(ADC0_BASE, 0);
}

/**
 * Initialize the analog input
 */
void adc_ain_init(void)
{
    // set pin PE0 to read the output of the current sensor
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
}

/**
 * Get current sensor and mcu temperatures
 */
void adc_get_measurements(uint16_t * current, uint16_t * mcu_temp)
{
    uint32_t data[2];

    ADCSequenceDataGet(ADC0_BASE, 0, data);

    *current = (uint16_t)data[0];
    *mcu_temp = (uint16_t)data[1];
}

inline void adc_start_measurement(void)
{
    ADCProcessorTrigger(ADC0_BASE, 0);
}
