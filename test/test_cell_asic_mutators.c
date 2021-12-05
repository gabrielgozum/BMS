/**
 * @brief Test cell_asic_mutators.c
 *
 * @par This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include "unity.h"
#include "cell_asic_mutators.h"

#include "bms.h"
#include "ltc6811.h"

cell_asic bms_ic[TOTAL_IC];

void setUp(void)
{

}

void tearDown(void)
{

}

void test_cam_set_cell_voltage_TestIndividualCells(void)
{
    // test cell 0
    cam_set_cell_voltage(bms_ic, 4095, 0); // add offset
    TEST_ASSERT_EQUAL_UINT16(4095 + TEST_CELL_0_VOLTAGE_OFFSET, bms_ic[1].cells.c_codes[0]);

    cam_set_cell_voltage(bms_ic, 0, 0);
    TEST_ASSERT_EQUAL_UINT16(0 + TEST_CELL_0_VOLTAGE_OFFSET, bms_ic[1].cells.c_codes[0]);

    // test cell 14
    cam_set_cell_voltage(bms_ic, 4095, 14);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[3].cells.c_codes[4]);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[3].cells.c_codes[5]);

    cam_set_cell_voltage(bms_ic, 0, 14);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[3].cells.c_codes[4]);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[3].cells.c_codes[5]);

    // test cell 59
    cam_set_cell_voltage(bms_ic, 4095, 59);
    TEST_ASSERT_EQUAL_UINT16(4095 + TEST_CELL_9_VOLTAGE_OFFSET, bms_ic[11].cells.c_codes[10]);
    TEST_ASSERT_EQUAL_UINT16(4095 + TEST_CELL_9_VOLTAGE_OFFSET, bms_ic[11].cells.c_codes[11]);

    cam_set_cell_voltage(bms_ic, 0, 59);
    TEST_ASSERT_EQUAL_UINT16(0 + TEST_CELL_9_VOLTAGE_OFFSET, bms_ic[11].cells.c_codes[10]);
    TEST_ASSERT_EQUAL_UINT16(0 + TEST_CELL_9_VOLTAGE_OFFSET, bms_ic[11].cells.c_codes[11]);
}

void test_cam_set_all_cell_voltages_TestAllCells(void)
{
    cam_set_all_cell_voltages(bms_ic, 100);

    uint8_t i;
    for  (i = 1; i < TOTAL_IC; i+=2)
    {
        uint8_t j;
        for (j = 0; j < 12; j++)
        {
            if (j == 0)
            {
                TEST_ASSERT_EQUAL_UINT16(100 + TEST_CELL_0_VOLTAGE_OFFSET, bms_ic[i].cells.c_codes[j]);
            }
            else if (j == 10 || j == 11)
            {
                TEST_ASSERT_EQUAL_UINT16(100 + TEST_CELL_9_VOLTAGE_OFFSET, bms_ic[i].cells.c_codes[j]);
            }
            else
            {
                TEST_ASSERT_EQUAL_UINT16(100, bms_ic[i].cells.c_codes[j]);
            }
        }
    }

    cam_set_all_cell_voltages(bms_ic, 56);
    for  (i = 1; i < TOTAL_IC; i+=2)
    {
        uint8_t j;
        for (j = 0; j < 12; j++)
        {
            if (j == 0)
            {
                TEST_ASSERT_EQUAL_UINT16(56 + TEST_CELL_0_VOLTAGE_OFFSET, bms_ic[i].cells.c_codes[j]);
            }
            else if (j == 10 || j == 11)
            {
                TEST_ASSERT_EQUAL_UINT16(56 + TEST_CELL_9_VOLTAGE_OFFSET, bms_ic[i].cells.c_codes[j]);
            }
            else
            {
                TEST_ASSERT_EQUAL_UINT16(56, bms_ic[i].cells.c_codes[j]);
            }
        }
    }
}

void test_cam_set_cell_temperature_TestIndividualCells(void)
{
    // test cell 0
    cam_set_cell_temperature(bms_ic, 4095, 0);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[0].cells.c_codes[0]);

    cam_set_cell_temperature(bms_ic, 0, 0);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[0].cells.c_codes[0]);

    // test cell 14
    cam_set_cell_temperature(bms_ic, 4095, 14);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[2].cells.c_codes[4]);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[2].cells.c_codes[5]);

    cam_set_cell_temperature(bms_ic, 0, 14);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[2].cells.c_codes[4]);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[2].cells.c_codes[5]);

    // test cell 59
    cam_set_cell_temperature(bms_ic, 4095, 59);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[10].cells.c_codes[10]);
    TEST_ASSERT_EQUAL_UINT16(4095, bms_ic[10].cells.c_codes[11]);

    cam_set_cell_temperature(bms_ic, 0, 59);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[10].cells.c_codes[10]);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[10].cells.c_codes[11]);
}

void test_cam_set_all_cell_temperature_TestAllCells(void)
{
    cam_set_all_cell_temperatures(bms_ic, 100);

    uint8_t i;
    for  (i = 0; i < TOTAL_IC; i+=2)
    {
        uint8_t j;
        for (j = 0; j < 12; j++)
        {
            TEST_ASSERT_EQUAL_UINT16(100, bms_ic[i].cells.c_codes[j]);
        }
    }

    cam_set_all_cell_temperatures(bms_ic, 56);
    for  (i = 0; i < TOTAL_IC; i+=2)
    {
        uint8_t j;
        for (j = 0; j < 12; j++)
        {
            TEST_ASSERT_EQUAL_UINT16(56, bms_ic[i].cells.c_codes[j]);
        }
    }
}

void test_cam_set_bsbt_ratio(void)
{
    // test bsb 0
    cam_set_bsbt_ratio(bms_ic, 100, 0);
    TEST_ASSERT_EQUAL_UINT16(100, bms_ic[0].aux.a_codes[0]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[0].stat.stat_codes[2]);

    cam_set_bsbt_ratio(bms_ic, 0, 0);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[0].aux.a_codes[0]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[0].stat.stat_codes[2]);

    // test bsb 1
    cam_set_bsbt_ratio(bms_ic, 10, 1);
    TEST_ASSERT_EQUAL_UINT16(10, bms_ic[2].aux.a_codes[0]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[2].stat.stat_codes[2]);

    cam_set_bsbt_ratio(bms_ic, 43, 1);
    TEST_ASSERT_EQUAL_UINT16(43, bms_ic[2].aux.a_codes[0]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[2].stat.stat_codes[2]);

    // test bsb 6
    cam_set_bsbt_ratio(bms_ic, 88, 6);
    TEST_ASSERT_EQUAL_UINT16(88, bms_ic[12].aux.a_codes[0]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[12].stat.stat_codes[2]);

    cam_set_bsbt_ratio(bms_ic, 2, 6);
    TEST_ASSERT_EQUAL_UINT16(2, bms_ic[12].aux.a_codes[0]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[12].stat.stat_codes[2]);
}

void test_cam_set_bsbh_ratio(void)
{
    // test bsb 0
    cam_set_bsbh_ratio(bms_ic, 100, 0);
    TEST_ASSERT_EQUAL_UINT16(100, bms_ic[0].aux.a_codes[1]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[0].stat.stat_codes[2]);

    cam_set_bsbh_ratio(bms_ic, 0, 0);
    TEST_ASSERT_EQUAL_UINT16(0, bms_ic[0].aux.a_codes[1]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[0].stat.stat_codes[2]);

    // test bsb 3
    cam_set_bsbh_ratio(bms_ic, 10, 3);
    TEST_ASSERT_EQUAL_UINT16(10, bms_ic[6].aux.a_codes[1]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[6].stat.stat_codes[2]);

    cam_set_bsbh_ratio(bms_ic, 43, 3);
    TEST_ASSERT_EQUAL_UINT16(43, bms_ic[6].aux.a_codes[1]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[6].stat.stat_codes[2]);

    // test bsb 6
    cam_set_bsbh_ratio(bms_ic, 88, 6);
    TEST_ASSERT_EQUAL_UINT16(88, bms_ic[12].aux.a_codes[1]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[12].stat.stat_codes[2]);

    cam_set_bsbh_ratio(bms_ic, 2, 6);
    TEST_ASSERT_EQUAL_UINT16(2, bms_ic[12].aux.a_codes[1]);
    TEST_ASSERT_EQUAL_UINT16(99, bms_ic[12].stat.stat_codes[2]);
}
