#include <stdint.h>
#include <stdbool.h>

void SysTickPeriodSet(uint32_t period)
{
    period++;
}

void SysCtlClockGet(void)
{
}

void SysTickEnable(void)
{
}

void SysTickIntEnable(void)
{
}

void SysCtlPeripheralEnable(uint32_t ui32Peripheral)
{
    ui32Peripheral++;
}

bool SysCtlPeripheralReady(uint32_t ui32Peripheral)
{
    ui32Peripheral++;
    return true;
}

void TimerConfigure(uint32_t ui32Base, uint32_t ui32Config)
{
    ui32Base++;
    ui32Config++;
}

void TimerLoadSet(uint32_t ui32Base, uint32_t ui32Timer, uint32_t ui32Value)
{
    ui32Base++;
    ui32Timer++;
    ui32Value++;
}

void TimerIntClear(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    ui32Base++;
    ui32IntFlags++;
}

void IntPrioritySet(uint32_t ui32Interrupt, uint8_t ui8Priority)
{
    ui32Interrupt++;
    ui8Priority++;
}

void TimerIntEnable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    ui32Base++;
    ui32IntFlags++;
}

void IntEnable(uint32_t ui32Interrupt)
{
    ui32Interrupt++;
}

void TimerEnable(uint32_t ui32Base, uint32_t ui32Timer)
{
    ui32Base++;
    ui32Timer++;
}

void SysCtlDelay(uint32_t period)
{
    period++;
}
