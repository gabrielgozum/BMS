/**
 * @file test_scheduler.c
 *
 * @brief Test scheduler.c
 *
 * This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include <stdbool.h>
#include <stdint.h>

#include "unity.h"
#include "scheduler.h"

void helper_1(void);
void helper_2(void);
void helper_inactive(void);


scheduler_task_t scheduler_table[] =
        {
         {helper_1, 0, 0, true},
         {helper_2, 10, 0, true},
         {helper_inactive, 1, 0, false}
        };


uint16_t scheduler_length = sizeof(scheduler_table) / sizeof(scheduler_task_t);

uint16_t count_1 = 0;
uint16_t count_2 = 0;
uint16_t inactive_count = 0;


static const uint32_t DURATION = 5000;

void setUp(void)
{
    // Don't need to set up, not using systick.
}


void tearDown(void)
{

}


void test_scheduler(void)
{
    uint32_t cnt;

    // reset last called
    uint32_t ticks = scheduler_ticks_get();
    for (cnt = 0; cnt < scheduler_length; cnt++)
    {
        scheduler_table[cnt].last_called = ticks;
    }

    for (cnt = 0; cnt < DURATION; cnt++)
    {
        // simulate systick every 2 cnt
        if (cnt % 2 == 0)
        {
            scheduler_int_handler();
        }
        scheduler_run();
    }

    TEST_ASSERT_EQUAL_UINT16(5000, count_1);
    TEST_ASSERT_EQUAL_UINT16(250, count_2);
    TEST_ASSERT_EQUAL_UINT16(0, inactive_count);
}


/**
 * There's possibly some off by one error in this test for how many times
 * each function is called in the scheduler.
 */
void test_TestEnableDisableFunctions(void)
{
    uint32_t cnt;

    // reset task counts
    count_1 = 0;
    count_2 = 0;

    // reset last called
    uint32_t ticks = scheduler_ticks_get();
    for (cnt = 0; cnt < scheduler_length; cnt++)
    {
        scheduler_table[cnt].last_called = ticks;
    }

    for (cnt = 0; cnt < DURATION; cnt++)
    {
        // simulate systick every 2 cnt
        if (cnt % 2 == 0)
        {
            scheduler_int_handler();
        }
        scheduler_run();
    }

    TEST_ASSERT_EQUAL_UINT16(5000, count_1);
    TEST_ASSERT_EQUAL_UINT16(250, count_2);

    // disable task 0
    scheduler_task_disable(helper_1);

    for (cnt = 0; cnt < DURATION; cnt++)
    {
        // simulate systick every 2 cnt
        if (cnt % 2 == 0)
        {
            scheduler_int_handler();
        }
        scheduler_run();
    }

    TEST_ASSERT_EQUAL_UINT16(5000, count_1);
    TEST_ASSERT_EQUAL_UINT16(500, count_2);

    // re-enable task 0 and set to run right away
    scheduler_task_enable(helper_1, true);

    for (cnt = 0; cnt < DURATION; cnt++)
    {
        // simulate systick every 2 cnt
        if (cnt % 2 == 0)
        {
            scheduler_int_handler();
        }
        scheduler_run();
    }

    TEST_ASSERT_EQUAL_UINT16(10000, count_1);
    TEST_ASSERT_EQUAL_UINT16(750, count_2);

    // disable task 1
    scheduler_task_disable(helper_2);

    for (cnt = 0; cnt < DURATION; cnt++)
    {
        // simulate systick every 2 cnt
        if (cnt % 2 == 0)
        {
            scheduler_int_handler();
        }
        scheduler_run();
    }

    TEST_ASSERT_EQUAL_UINT16(15000, count_1);
    TEST_ASSERT_EQUAL_UINT16(750, count_2);

    // re-enable task 1 and set it to run on the next cycle
    scheduler_task_enable(helper_2, false);

    for (cnt = 0; cnt < DURATION; cnt++)
    {
        // simulate systick every 2 cnt
        if (cnt % 2 == 0)
        {
            scheduler_int_handler();
        }
        scheduler_run();
    }

    TEST_ASSERT_EQUAL_UINT16(20000, count_1);
    TEST_ASSERT_EQUAL_UINT16(1000, count_2);
}

void helper_1(void)
{
    count_1++;
}

void helper_2(void)
{
    count_2++;
}

void helper_inactive(void)
{
    inactive_count++;
}

