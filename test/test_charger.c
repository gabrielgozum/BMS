/**
 * @brief Test charger.c
 *
 * This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include <stdbool.h>
#include <stdint.h>

#include "unity.h"
#include "mock/Mockdtc.h"
#include "mock/Mocksdc_control.h"

#include "bms.h"
#include "charger.h"

bms_t bms;
charger_t charger;

void setUp(void)
{
    dtc_get_update_lp_IgnoreAndReturn(false);
    bms_bms_init(&bms);
    charger_t_init(&charger);

    bms.airs_closed = 1;
}

void tearDown(void)
{

}

void test_charger_t_init_TestInit(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, charger.input_voltage);
    TEST_ASSERT_EQUAL_UINT8(0, charger.output_voltage);
    TEST_ASSERT_EQUAL_UINT8(0, charger.input_current_limit_max);
    TEST_ASSERT_EQUAL_UINT8(0, charger.input_current);
    TEST_ASSERT_EQUAL_INT8(0, charger.temperature);
    TEST_ASSERT_EQUAL_UINT8(3, charger.ignition_status);
    TEST_ASSERT_EQUAL_UINT8(7, charger.charger_state);
    TEST_ASSERT_EQUAL_UINT8(7, charger.fault_severity);

    TEST_ASSERT_FALSE(charger.rx_msg_checksum_faulted);
    TEST_ASSERT_FALSE(charger.rx_msg_counter_faulted);

    TEST_ASSERT_EQUAL_UINT16(0, charger.output_current);
    TEST_ASSERT_EQUAL_UINT8(3, charger.charge_complete);
    TEST_ASSERT_EQUAL_UINT8(0, charger.charge_enable);
    TEST_ASSERT_EQUAL_UINT8(0, charger.charge_system_fault);

    TEST_ASSERT_EQUAL_UINT8(0, charger.charging_mode);
    TEST_ASSERT_EQUAL_UINT16(0, charger.time_limit);
}

/**
 * @test
 * Test the default charger voltage and current limits.
 */
void test_charger_set_command_values_DefaultCommand(void)
{
    // charger is disabled by default so need to enable
    charger.charge_enable = CHARGER_ENABLED;
    // bms ccl is 0 by default so need to enable
    bms.ccl = 60;

    // default command
    charger_set_command_values();

    TEST_ASSERT_EQUAL_UINT16(1248, charger.voltage_limit);
    TEST_ASSERT_EQUAL_UINT16(320, charger.current_limit);
}

/**
 * @test
 * Test that the charger sets the charger voltage and current limits to zero if
 * the charger becomes disabled.
 */
void test_charger_set_command_values_ChargerDisabled(void)
{
    // charger by default is disabled

    sdc_control_open_Expect();
    charger_set_command_values();

    TEST_ASSERT_EQUAL_UINT16(0, charger.voltage_limit);
    TEST_ASSERT_EQUAL_UINT16(0, charger.current_limit);
}

/**
 * @test
 * Test that the charger sets the charger voltage and current limits to zero if
 * the charger time limit is reached. The SDC should also be opened.
 */
void test_charger_set_command_values_ChargerTimeout(void)
{
    // charger is disabled by default so need to enable
    charger.charge_enable = CHARGER_ENABLED;
    // bms ccl is 0 by default so need to enable
    bms.ccl = 60;

    // charger timeout is 120 seconds, the charger_set_commands function is
    // called every 100 ticks, the timeout value is incremented by 5 ticks each
    // call, so 120 sec * 1000 ticks per sec / 5 = 72000 calls to timeouts

    uint32_t num_function_calls = 0;
    for (num_function_calls = 0; num_function_calls < 71999; num_function_calls++)
    {
        charger_set_command_values();
    }

    TEST_ASSERT_EQUAL_UINT16(1248, charger.voltage_limit);
    TEST_ASSERT_EQUAL_UINT16(320, charger.current_limit);

    sdc_control_open_Expect();
    charger_set_command_values();

    TEST_ASSERT_EQUAL_UINT16(0, charger.voltage_limit);
    TEST_ASSERT_EQUAL_UINT16(0, charger.current_limit);
}

/**
 * @test
 * Test the charger command being limited by the charge current limit.
 */
void test_charger_set_command_values_TestCurrentLimit(void)
{
    // charger is disabled by default so need to enable
    charger.charge_enable = CHARGER_ENABLED;

    // set ccl right above the default charger current limit
    bms.ccl = 17;

    charger_set_command_values();
    TEST_ASSERT_EQUAL_UINT16(320, charger.current_limit);

    // set ccl right below the default charger current limit
    bms.ccl = 15;

    charger_set_command_values();
    TEST_ASSERT_EQUAL_UINT16(300, charger.current_limit);

}

void test_charger_process_rx_data_Normal(void)
{
    charger.fault_severity = CHARGER_FAULT_SEVERITY_NONE;

    charger_process_rx_data();

    TEST_ASSERT_FALSE(bms.cl_override.bit.charger_fault);
}

void test_charger_process_rx_data_Fault(void)
{
    charger.fault_severity = CHARGER_FAULT_SEVERITY_HARD_FAIL;

    dtc_send_general_Expect(CHARGER_FAULT_DTC, DTC_ACTIVE);

    charger_process_rx_data();

    TEST_ASSERT_TRUE(bms.cl_override.bit.charger_fault);
}
