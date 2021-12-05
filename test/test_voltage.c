/**
 * @brief Test voltage.c
 *
 * This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "unity.h"
#include "voltage.h"
#include "bms.h"
#include "cell_asic_mutators.h"
#include "mock/Mockdtc.h"

cell_asic bms_ic[TOTAL_IC];

bms_t bms;

void setUp(void)
{
    dtc_get_update_lp_IgnoreAndReturn(false);
    dtc_get_update_hp_IgnoreAndReturn(false);
    bms_bms_init(&bms);
    bms_set_driving_limits(&(bms.limits));
}

void tearDown(void)
{

}

uint16_t min(uint16_t a, uint16_t b)
{
    return (a < b) ? a : b;
}

uint16_t max(uint16_t a, uint16_t b)
{
    return (a > b) ? a : b;
}

void ccv_test_fault_all(uint16_t voltage, uint16_t new_dtc, uint16_t last_dtc)
{
    bms.ccv_stats.average = voltage; // Set the average, to avoid the inital outlier fault
    uint8_t cell;
    cam_set_all_cell_voltages(bms_ic, voltage);

    // Expect calls need to be in order for each function
    for (cell = 0; cell < TOTAL_CELLS; cell++)
    {
        bms.ccv_stats.fault_codes[cell] = last_dtc; // Setup struct with last fault, as it gets cleared each test
        if (new_dtc != CCV_DTC)
        {
            dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, cell, voltage);
        }

        if (last_dtc != CCV_DTC)
        {
            dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, cell, voltage);
        }
    }

    vltg_calc_ccv_stats();

    TEST_ASSERT_EQUAL_HEX(new_dtc, bms.ccv_stats.fault_codes[2]); // check arbitrary cell
}

void ccv_test_fault_cell(uint8_t cell, uint16_t cell_voltage, uint16_t rest_voltage, uint16_t new_dtc, uint16_t last_dtc)
{
    bms.ccv_stats.average = rest_voltage; // Set the average, to avoid the inital outlier fault

    cam_set_all_cell_voltages(bms_ic, rest_voltage);
    cam_set_cell_voltage(bms_ic, cell_voltage, cell);

    if (new_dtc != CCV_DTC)
    {
        dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, cell, cell_voltage);
    }

    if (last_dtc != CCV_DTC)
    {
        bms.ccv_stats.fault_codes[cell] = last_dtc;
        dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, cell, cell_voltage);
    }

    vltg_calc_ccv_stats();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.ccv_stats.fault_codes[cell]);
    bms.ccv_stats.fault_codes[cell] = CCV_DTC; // reset fault codes
}


void test_ccv_OORL_all(void)
{
    ccv_test_fault_all(900, CCV_OORL_DTC, CCV_DTC); // OORL
    ccv_test_fault_all(805, CCV_OORL_DTC, CCV_UNDER_DTC); // OORL

    TEST_ASSERT_TRUE(bms.voltage_failsafe);
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT((uint16_t)0xffffffff, bms.ccv_stats.min);
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.sum);
}

void test_ccv_OORL_ind(void)
{
    ccv_test_fault_cell(12, 950, 30000, CCV_OORL_DTC, CCV_DTC);
    ccv_test_fault_cell(55, 900, 29000, CCV_OORL_DTC, CCV_HIGH_DTC);

    TEST_ASSERT_EQUAL_UINT(59, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.average);
    TEST_ASSERT_TRUE(bms.voltage_failsafe);

    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

void test_ccv_OORH_all(void)
{
    ccv_test_fault_all(60030, CCV_OORH_DTC, CCV_DTC); //OORH
    ccv_test_fault_all(60050, CCV_OORH_DTC, CCV_OORL_DTC); //OORH

    TEST_ASSERT_TRUE(bms.voltage_failsafe);
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT((uint16_t)0xffffffff, bms.ccv_stats.min);
    TEST_ASSERT_EQUAL_UINT(0, bms.ccv_stats.sum);

}

void test_ccv_OORH_ind(void)
{
    ccv_test_fault_cell(0, 60030, 30000, CCV_OORH_DTC, CCV_DTC);
    ccv_test_fault_cell(59, 60050, 29000, CCV_OORH_DTC, CCV_HIGH_DTC);

    TEST_ASSERT_TRUE(bms.voltage_failsafe);
    TEST_ASSERT_EQUAL_UINT(59, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

void test_ccv_UNDER_all(void)
{
    ccv_test_fault_all(16000, CCV_UNDER_DTC, CCV_DTC); // UNDER
    ccv_test_fault_all(19999, CCV_UNDER_DTC, CCV_OORH_DTC); // UNDER

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(19999, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(19999, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(19999, bms.ccv_stats.min);
    TEST_ASSERT_EQUAL_UINT(19999 / 100.0 * 60, bms.ccv_stats.sum);
}

void test_ccv_UNDER_ind(void)
{
    ccv_test_fault_cell(44, 16000, 30000, CCV_UNDER_DTC, CCV_DTC);
    ccv_test_fault_cell(30, 19999, 29000, CCV_UNDER_DTC, CCV_HIGH_DTC);

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT((29000 * 59 + 19999) / 60, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(19999, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    total += 19999 / 100;
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

void test_ccv_LOW_all(void)
{
    ccv_test_fault_all(26500, CCV_LOW_DTC, CCV_DTC); // LOW
    ccv_test_fault_all(26000, CCV_LOW_DTC, CCV_UNDER_DTC); // LOW

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(26000, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(26000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(26000, bms.ccv_stats.min);
    TEST_ASSERT_EQUAL_UINT(26000 / 100.0 * 60, bms.ccv_stats.sum);
}

void test_ccv_LOW_ind(void)
{
    ccv_test_fault_cell(23, 26000, 30000, CCV_LOW_DTC, CCV_DTC);
    ccv_test_fault_cell(14, 27000, 29000, CCV_LOW_DTC, CCV_HIGH_DTC);

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT((29000 * 59 + 27000) / 60, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(27000, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    total += 27000 / 100;
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

void test_ccv_OVER_all(void)
{
    ccv_test_fault_all(50021, CCV_OVER_DTC, CCV_DTC); // OVER
    ccv_test_fault_all(50000, CCV_OVER_DTC, CCV_LOW_DTC); // OVER

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(50000, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(50000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(50000, bms.ccv_stats.min);
    TEST_ASSERT_EQUAL_UINT(50000 / 100.0 * 60, bms.ccv_stats.sum);
}

void test_ccv_OVER_ind(void)
{
    ccv_test_fault_cell(54, 50021, 30000, CCV_OVER_DTC, CCV_DTC);
    ccv_test_fault_cell(32, 50000, 29000, CCV_OVER_DTC, CCV_HIGH_DTC);

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT((29000 * 59 + 50000) / 60, bms.ccv_stats.average);
    
    TEST_ASSERT_EQUAL_UINT(50000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    total += 50000 / 100;
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

/*
 * @test
 * Check the CCV High fault is sent if all the cell voltgaes
 * are above the threshold.
 */
void test_ccv_HIGH_all(void)
{
    ccv_test_fault_all(41000, CCV_HIGH_DTC, CCV_DTC); // HIGH
    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
}

/*
 * @test
 * Set one cell voltage high and check a fault is sent.
 */
void test_ccv_HIGH_ind(void)
{
    ccv_test_fault_cell(11, 41000, 30000, CCV_HIGH_DTC, CCV_DTC);
    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
}

void test_ccv_OUTLIER_ind(void)
{
    ccv_test_fault_cell(37, 28010, 35060, CCV_OUTLIER_DTC, CCV_DTC);
    ccv_test_fault_cell(45, 36050, 29000, CCV_OUTLIER_DTC, CCV_HIGH_DTC);

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT((29000 * 59 + 36050) / 60, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(36050, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    total += 36050 / 100;
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

void test_ccv_NORMAL_all(void)
{
    ccv_test_fault_all(28500, CCV_DTC, CCV_DTC); // NORMAL
    ccv_test_fault_all(30000, CCV_DTC, CCV_HIGH_DTC); // NORMAL

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(30000, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(30000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(30000, bms.ccv_stats.min);
    TEST_ASSERT_EQUAL_UINT(30000 / 100.0 * 60, bms.ccv_stats.sum);
}

void test_ccv_NORMAL_ind(void)
{
    ccv_test_fault_cell(3, 29000, 30000, CCV_DTC, CCV_DTC);
    ccv_test_fault_cell(6, 28050, 29000, CCV_DTC, CCV_OUTLIER_DTC);

    TEST_ASSERT_FALSE(bms.voltage_failsafe);

    TEST_ASSERT_EQUAL_UINT(60, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT((29000 * 59 + 28050) / 60, bms.ccv_stats.average);

    TEST_ASSERT_EQUAL_UINT(29000, bms.ccv_stats.max);
    TEST_ASSERT_EQUAL_UINT(28050, bms.ccv_stats.min);

    uint16_t total = 29000 / 100 * (TOTAL_CELLS - 1);
    total += 28050 / 100;
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}

/* open wire test disabled
void test_ccv_open_wire_ind(void)
{
    bms_ic[2].system_open_wire = 0x0001;

    bms.ccv_stats.average = 30000; // Set the average, to avoid the inital outlier fault

    cam_set_all_cell_voltages(bms_ic, 30000);

    dtc_send_cell_Expect(CCV_OPEN_WIRE_DTC, DTC_ACTIVE, 10);

    vltg_calc_ccv_stats();

    TEST_ASSERT_EQUAL_UINT(CCV_OPEN_WIRE_DTC, bms.ccv_stats.fault_codes[10]);
    TEST_ASSERT_TRUE(bms.voltage_failsafe);

    TEST_ASSERT_EQUAL_UINT(59, bms.ccv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT(30000, bms.ccv_stats.average);

    uint16_t total = 30000 / 100 * (TOTAL_CELLS - 1);
    TEST_ASSERT_EQUAL_UINT(total, bms.ccv_stats.sum);
}
*/

extern uint32_t inc_num_valid_cells;
void test_ocv_calc_ch(void)
{
    vltg_reset_inc_stats();
    inc_num_valid_cells = 60;

    bms.i_pack = -1;
    bms.is_idle = false;
    uint8_t i;
    for (i = 0; i < TOTAL_CELLS; i++)
    {
        bms.ccv_stats.voltage[i] = 10;
        bms.cr_ch_stats.cr[i] = -5;
    }

    uint8_t ch;
    for (ch = 1; ch <= 5; ch++)
    {
        vltg_calc_ocv_ch(ch);
    }
    vltg_store_ocv_stats();

    for (i = 0; i < TOTAL_CELLS; i++)
    {
        TEST_ASSERT_EQUAL_UINT16(15, bms.ocv_stats.voltage[i]);
    }

    TEST_ASSERT_EQUAL_UINT16(15, bms.ocv_stats.max);
    TEST_ASSERT_EQUAL_UINT16(15, bms.ocv_stats.min);
    TEST_ASSERT_EQUAL_UINT16(15, bms.ocv_stats.average);
    TEST_ASSERT_EQUAL_UINT16(60, bms.ocv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT16(60 * 15 / 100, bms.ocv_stats.sum);

    vltg_reset_inc_stats();
    inc_num_valid_cells = 60;

    bms.i_pack = 20;
    for (i = 0; i < TOTAL_CELLS; i++)
    {
        bms.ccv_stats.voltage[i] = 4000;
        bms.cr_dch_stats.cr[i] = 555;
    }

    for (ch = 1; ch <= 5; ch++)
    {
        vltg_calc_ocv_ch(ch);
    }
    vltg_store_ocv_stats();

    for (i = 0; i < TOTAL_CELLS; i++)
    {
        TEST_ASSERT_EQUAL_UINT16(15100, bms.ocv_stats.voltage[i]);
    }

    TEST_ASSERT_EQUAL_UINT16(15100, bms.ocv_stats.max);
    TEST_ASSERT_EQUAL_UINT16(15100, bms.ocv_stats.min);
    TEST_ASSERT_EQUAL_UINT16(15100, bms.ocv_stats.average);
    TEST_ASSERT_EQUAL_UINT16(60, bms.ocv_stats.valid_cells);
    TEST_ASSERT_EQUAL_UINT16(60 * 15100 / 100, bms.ocv_stats.sum);
}
