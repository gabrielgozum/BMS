/**
 * @brief Interface for timers.
 *
 * This file is part of a CCS project and intended to be built with CCS
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_timer.h"
#include "driverlib/interrupt.h"
#include "Lib_rgb.h"
#include "timers.h"


void timer0_init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {};

    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);

    //Enable interrupts
    IntEnable(INT_TIMER0A);

    //IntPrioritySet(TIMER0_BASE, 1);

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void timer1_init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {};

    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);

    //Enable interrupts
    IntEnable(INT_TIMER1A);

    //IntPrioritySet(TIMER1_BASE, 1);

    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}
