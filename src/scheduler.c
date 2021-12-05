/**
 * @file scheduler.c
 *
 * @brief Contains implementation of timer-based scheduling operation.
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "scheduler.h"

// Variables to track current time in ticks
static volatile uint32_t scheduler_ticks = 0;

uint32_t elapsed_time = 0;
extern uint16_t scheduler_length;

//static void timer0A_init(uint32_t ticks_per_second)
//{
//    // Enable clocking and wait for it to be ready
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
//    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {};
//
//    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
//
//
//    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / ticks_per_second);
//
//    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
//    IntPrioritySet(INT_TIMER0A, 0);
//}



void scheduler_init(uint32_t ticks_per_second)
{
    SysTickPeriodSet(SysCtlClockGet() / ticks_per_second);
    SysTickEnable();
    SysTickIntEnable();
}


void scheduler_run(void)
{
    uint32_t cnt;
    scheduler_task_t * curr_task;

    for(cnt = 0; cnt < scheduler_length; cnt++)
    {
        curr_task = &scheduler_table[cnt];


        if (curr_task->active && scheduler_elapsed_ticks(curr_task->last_called) >= curr_task->period)
        {
            if (cnt == 13)
                        {
                            elapsed_time = scheduler_elapsed_ticks(curr_task->last_called);
            }
            curr_task->last_called = scheduler_ticks;
            curr_task->p_function();
        }
    }
}


uint32_t scheduler_ticks_get(void)
{
    return scheduler_ticks;
}


uint32_t scheduler_elapsed_ticks(uint32_t count)
{
    return scheduler_elapsed_ticks_calc(count, scheduler_ticks);
}


uint32_t scheduler_elapsed_ticks_calc(uint32_t start, uint32_t end)
{
    return((end > start) ? (end - start) :
           ((0xFFFFFFFF - start) + end + 1));
}


void scheduler_int_handler(void)
{
    scheduler_ticks++; // Only lp ticks handled in this loop. Keep HP ticks in sync
}


void scheduler_task_enable(void (*p_function)(void), bool run_now)
{
    uint32_t task_i;
    for (task_i = 0; task_i < scheduler_length; task_i++)
    {
        if (scheduler_table[task_i].p_function == p_function)
        {
            scheduler_table[task_i].active = true;

            // set the last call time to ensure the function is called on the next
            // call to the scheduler or after a desired number of ticks
            if (run_now)
            {
                // cause task to run on the next call to the scheduler
                scheduler_table[task_i].last_called = (scheduler_ticks
                        - scheduler_table[task_i].period);
            }
            else
            {
                // cause the task to run after one full time period
                scheduler_table[task_i].last_called = scheduler_ticks;
            }
        }
    }
}


void scheduler_task_disable(void (*p_function)(void))
{
    uint32_t task_i;
    for (task_i = 0; task_i < scheduler_length; task_i++)
    {
        if (scheduler_table[task_i].p_function == p_function)
        {
            scheduler_table[task_i].active = false;
            break;
        }
    }
}

