/**
 * @brief Test can.c
 *
 * @par This file is part of a makefile project and is intended to be built on
 * Linux.
 */
#include <stdbool.h>
#include <stdint.h>

#include "unity.h"
#include "can.h"

#include "bms.h"
#include "charger.h"

bms_t bms;
charger_t charger;

void setUp(void)
{
    bms_bms_init(&bms);
    bms_set_driving_limits(&(bms.limits));
    charger_t_init(&charger);
}

void tearDown(void)
{

}

void test_can_calculate_charger_checksum_TestValues(void)
{
    uint8_t data[8];

    uint32_t i = 0;
    for (i = 0; i < 8; i++)
    {
        data[i] = 0;
    }

    TEST_ASSERT_EQUAL_UINT8(7, can_calc_chrgr_checksum(data, CAN_CHARGER_CMD_MSG_ID));

    data[0] = 1;
    TEST_ASSERT_EQUAL_UINT8(0, can_calc_chrgr_checksum(data, CAN_CHARGER_CMD_MSG_ID));

    data[1] = 1;
    TEST_ASSERT_EQUAL_UINT8(1, can_calc_chrgr_checksum(data, CAN_CHARGER_CMD_MSG_ID));

    for (i = 0; i < 8; i++)
    {
        data[i] = 0xef;
    }

    TEST_ASSERT_EQUAL_UINT8(5, can_calc_chrgr_checksum(data, CAN_CHARGER_CMD_MSG_ID));
}

void test_can_is_rx_charger_checksum_valid_TestValidCounterValues(void)
{
    TEST_IGNORE();
}

void test_can_is_rx_charger_checksum_valid_TestInvalidCounterValues(void)
{
    TEST_IGNORE();
}

void test_can_is_rx_charger_checksum_valid_TestInvalidChecksumValues(void)
{
    TEST_IGNORE();
}

void test_can_read_charger_status_TestValidData(void)
{
    TEST_IGNORE();
}

void test_can_read_charger_status_TestInvalidData(void)
{
    TEST_IGNORE();
}

