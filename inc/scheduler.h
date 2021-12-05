/**
 * @brief Contains implementation of timer-based scheduling operation.
 *
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include "stdbool.h"

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------
#define TICKS_PER_SECOND                    1000 //!< 1ms ticks

//-----------------------------------------------------------------------------
// Scheduler Periods
//-----------------------------------------------------------------------------
#define UPDATE_SOC_PERIOD                           5   //!< [ticks]
#define MAIN_UPDATE_MEASUREMENTS_PRIMARY_PERIOD     100 //!< [ticks]
#define CHARGER_COMMAND_PERIOD                      100 //!< [ticks]

typedef struct
{
    void (*p_function)(void); // Pointer to function to call (no argument, no return type)

    uint32_t period; // How often task is called

    uint32_t last_called; // systick when function last called.

    bool active;
} scheduler_task_t;

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------

// Use variables declared in main file.
extern scheduler_task_t scheduler_table[];
extern uint16_t scheduler_length;


/**
 * Initialize the systick timer. The systick timer increments the low priority
 * scheduler ticks count.
 *
 * @param ticks_per_second Frequency of ticks
 */
void scheduler_init(uint32_t ticks_per_second);


void scheduler_int_handler(void);


void scheduler_run(void);


/**
 * Enables a task on the lp table of the scheduler.
 * @param num_tasks The number of tasks on the schedule the task is on
 * @param task_index Index of the task.
 * @param run_now Should the task run right now.
 */
void scheduler_task_enable(void (*p_function)(void), bool run_now);


void scheduler_task_disable(void (*p_function)(void));


uint32_t scheduler_ticks_get(void);


uint32_t scheduler_elapsed_ticks(uint32_t count);


uint32_t scheduler_elapsed_ticks_calc(uint32_t start, uint32_t end);


#endif /* INC_SCHEDULER_H_ */
