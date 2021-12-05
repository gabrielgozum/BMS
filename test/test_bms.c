/**
 * @brief Unit tests for bms.c
 *
 * This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include <stdbool.h>
#include <stdint.h>
#include "unity.h"
#include "params.h"
#include "bms.h"

bms_t bms;

void setUp(void)
{
    bms_bms_init(&bms);
    bms_set_driving_limits(&(bms.limits));
}

void tearDown(void)
{

}

void test_bms_cell_to_bsb_id_ct_TestInRangeValues(void)
{
    uint8_t cell_id = 0;
    uint8_t ic_id;
    uint8_t c_i;

    bms_cell_to_bsb_id_ct(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_UINT8(0, ic_id);
    TEST_ASSERT_EQUAL_UINT8(0, c_i);

    cell_id = 59;
    bms_cell_to_bsb_id_ct(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_UINT8(10, ic_id);
    TEST_ASSERT_EQUAL_UINT8(10, c_i);

    cell_id = 37;
    bms_cell_to_bsb_id_ct(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_UINT8(6, ic_id);
    TEST_ASSERT_EQUAL_UINT8(8, c_i);
}

void test_bms_cell_to_bsb_id_ct_TestOutOfRangeValues(void)
{
    uint8_t cell_id = 100;
    uint8_t ic_id;
    uint8_t c_i;

    bms_cell_to_bsb_id_ct(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_HEX(0xff, ic_id);
    TEST_ASSERT_EQUAL_HEX(0xff, c_i);

    cell_id = 60;
    bms_cell_to_bsb_id_ct(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_HEX(0xff, ic_id);
    TEST_ASSERT_EQUAL_HEX(0xff, c_i);
}

void test_bms_cell_to_bsb_id_ccv_TestInRangeValues(void)
{
    uint8_t cell_id = 0;
    uint8_t ic_id;
    uint8_t c_i;

    bms_cell_to_bsb_id_ccv(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_UINT8(1, ic_id);
    TEST_ASSERT_EQUAL_UINT8(0, c_i);

    cell_id = 59;
    bms_cell_to_bsb_id_ccv(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_UINT8(11, ic_id);
    TEST_ASSERT_EQUAL_UINT8(10, c_i);

    cell_id = 37;
    bms_cell_to_bsb_id_ccv(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_UINT8(7, ic_id);
    TEST_ASSERT_EQUAL_UINT8(8, c_i);
}

void test_bms_cell_to_bsb_id_ccv_TestOutOfRangeValues(void)
{
    uint8_t cell_id = 100;
    uint8_t ic_id;
    uint8_t c_i;

    bms_cell_to_bsb_id_ccv(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_HEX(0xff, ic_id);
    TEST_ASSERT_EQUAL_HEX(0xff, c_i);

    cell_id = 60;
    bms_cell_to_bsb_id_ccv(cell_id, &ic_id, &c_i);
    TEST_ASSERT_EQUAL_HEX(0xff, ic_id);
    TEST_ASSERT_EQUAL_HEX(0xff, c_i);
}

void test_bms_ch_to_cell_id_TestInRangeValues(void)
{
    uint8_t ch = 1;
    uint8_t ic_id = 0;
    uint8_t cells[2];

    bms_ch_to_cell_ids(ch, ic_id, cells);
    TEST_ASSERT_EQUAL_UINT8(0, cells[0]);
    TEST_ASSERT_EQUAL_UINT8(5, cells[1]);

    ch = 5;
    ic_id = 11;
    bms_ch_to_cell_ids(ch, ic_id, cells);
    TEST_ASSERT_EQUAL_UINT8(54, cells[0]);
    TEST_ASSERT_EQUAL_UINT8(59, cells[1]);

    ch = 3;
    ic_id = 4;
    bms_ch_to_cell_ids(ch, ic_id, cells);
    TEST_ASSERT_EQUAL_UINT8(22, cells[0]);
    TEST_ASSERT_EQUAL_UINT8(27, cells[1]);
}

void test_bms_ch_to_cell_id_TestOutOfRangeValues(void)
{
    uint8_t ch = 0;
    uint8_t ic_id = 0;
    uint8_t cells[2];

    bms_ch_to_cell_ids(ch, ic_id, cells);
    TEST_ASSERT_EQUAL_UINT8(0xff, cells[0]);
    TEST_ASSERT_EQUAL_UINT8(0xff, cells[1]);

    ch = 6;
    ic_id = 5;
    bms_ch_to_cell_ids(ch, ic_id, cells);
    TEST_ASSERT_EQUAL_UINT8(0xff, cells[0]);
    TEST_ASSERT_EQUAL_UINT8(0xff, cells[1]);

    ch = 3;
    ic_id = 12;
    bms_ch_to_cell_ids(ch, ic_id, cells);
    TEST_ASSERT_EQUAL_UINT8(0xff, cells[0]);
    TEST_ASSERT_EQUAL_UINT8(0xff, cells[1]);
}

//****************************************************************************
// bms_cr_calc
//****************************************************************************

/**
 * @test
 * Test the online resistance calculation.
 */
void test_bms_cr_calc_Test(void)
{
    TEST_IGNORE(); // currently unused
}

//****************************************************************************
// bms_lookup_cr
//****************************************************************************

/**
 * @test
 * Test the statistics calculated by the cell resistance lookup function.
 */
void test_bms_lookup_cr_TestStatsAllTheSame(void)
{
    bms.soc = 0.5;

    uint8_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        bms.ct_stats.ct[cell_i] =  25;
    }

    bms_lookup_cr();

    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_ch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_ch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_ch_stats.average);

    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_dch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_dch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_dch_stats.average);

    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        bms.ct_stats.ct[cell_i] = 31;
    }

    bms_lookup_cr();
    
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_ch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_ch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_ch_stats.average);

    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_dch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_dch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_dch_stats.average);

    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        bms.ct_stats.ct[cell_i] = 47;
    }

    bms_lookup_cr();

    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_ch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_ch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_ch_stats.average);

    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_dch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_dch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_dch_stats.average);
}

/**
  * @test
  * Test how a low resistance affects the stats
  */
void test_bms_lookup_cr_TestStatsLowResistance(void)
{
    bms.soc = 0.5;

    uint8_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        bms.ct_stats.ct[cell_i] =  25;
    }
    
    bms.ct_stats.ct[5] = 55;
    
    bms_lookup_cr();
    
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_ch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_ch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_ch_stats.average);
    TEST_ASSERT_EQUAL_UINT8(5, bms.cr_ch_stats.min_id);

    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_dch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_dch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(23, bms.cr_dch_stats.average);
    TEST_ASSERT_EQUAL_UINT8(5, bms.cr_dch_stats.min_id);
}

/**
  * @test
  * Test how a high resistance affects the stats.
  */
void test_bms_lookup_cr_TestStatsHighResistance(void)
{
    bms.soc = 0.5;

    uint8_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        bms.ct_stats.ct[cell_i] =  25;
    }
    
    bms.ct_stats.ct[5] = 15;
    
    bms_lookup_cr();
    
    TEST_ASSERT_EQUAL_FLOAT(30, bms.cr_ch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_ch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_ch_stats.average);
    TEST_ASSERT_EQUAL_UINT8(5, bms.cr_ch_stats.max_id);

    TEST_ASSERT_EQUAL_FLOAT(29, bms.cr_dch_stats.max);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_dch_stats.min);
    TEST_ASSERT_EQUAL_FLOAT(24, bms.cr_dch_stats.average);
    TEST_ASSERT_EQUAL_UINT8(5, bms.cr_dch_stats.max_id);
}


/**
 * @test
 * Test charge and discharge cell resistance lookup values for various SOC and
 * cell temperature.
 */
void test_bms_lookup_cr_TestVariousValues(void)
{
    bms.soc = 0;
    bms.ct_stats.ct[2] =  15;
    bms.ct_stats.ct[59] = 20;
    bms.ct_stats.ct[21] = 55;
    bms_lookup_cr();

    // charge resistances
    TEST_ASSERT_EQUAL_FLOAT(48, bms.cr_ch_stats.cr[2]);
    TEST_ASSERT_EQUAL_FLOAT(41, bms.cr_ch_stats.cr[59]);
    TEST_ASSERT_EQUAL_FLOAT(26, bms.cr_ch_stats.cr[21]);

    // discharge resistances
    TEST_ASSERT_EQUAL_FLOAT(64, bms.cr_dch_stats.cr[2]);
    TEST_ASSERT_EQUAL_FLOAT(49, bms.cr_dch_stats.cr[59]);
    TEST_ASSERT_EQUAL_FLOAT(25, bms.cr_dch_stats.cr[21]);

    bms.soc = 0.5;
    bms_lookup_cr();

    // charge resistances
    TEST_ASSERT_EQUAL_FLOAT(30, bms.cr_ch_stats.cr[2]);
    TEST_ASSERT_EQUAL_FLOAT(27, bms.cr_ch_stats.cr[59]);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_ch_stats.cr[21]);

    // discharge resistances
    TEST_ASSERT_EQUAL_FLOAT(29, bms.cr_dch_stats.cr[2]);
    TEST_ASSERT_EQUAL_FLOAT(27, bms.cr_dch_stats.cr[59]);
    TEST_ASSERT_EQUAL_FLOAT(21, bms.cr_dch_stats.cr[21]);

    bms.soc = 1;
    bms_lookup_cr();

    // charge resistances
    TEST_ASSERT_EQUAL_FLOAT(87 , bms.cr_ch_stats.cr[2]);
    TEST_ASSERT_EQUAL_FLOAT(76 , bms.cr_ch_stats.cr[59]);
    TEST_ASSERT_EQUAL_FLOAT(51 , bms.cr_ch_stats.cr[21]);

    // discharge resistances
    TEST_ASSERT_EQUAL_FLOAT(38, bms.cr_dch_stats.cr[2]);
    TEST_ASSERT_EQUAL_FLOAT(33, bms.cr_dch_stats.cr[59]);
    TEST_ASSERT_EQUAL_FLOAT(22, bms.cr_dch_stats.cr[21]);
}

void test_bms_set_i_failsafe(void)
{
    bms_set_i_failsafe(true);
    TEST_ASSERT_TRUE(bms.i_failsafe);
    TEST_ASSERT_EQUAL_UINT16(CCV_MAX_LIM_FAILSAFE, bms.limits.ccv_max_limit);
    TEST_ASSERT_EQUAL_UINT16(CCV_MIN_LIM_FAILSAFE, bms.limits.ccv_min_limit);
    TEST_ASSERT_FALSE(bms.cl_override.bit.failsafe);

    bms_set_i_failsafe(false);
    TEST_ASSERT_FALSE(bms.i_failsafe);
    TEST_ASSERT_EQUAL_UINT16(CCV_MAX_LIM_NORMAL, bms.limits.ccv_max_limit);
    TEST_ASSERT_EQUAL_UINT16(CCV_MIN_LIM_NORMAL, bms.limits.ccv_min_limit);
    TEST_ASSERT_FALSE(bms.cl_override.bit.failsafe);
}

void test_bms_set_voltage_failsafe(void)
{
    bms_set_voltage_failsafe(true);
    TEST_ASSERT_TRUE(bms.voltage_failsafe);
    TEST_ASSERT_EQUAL_UINT16(CCV_MAX_LIM_FAILSAFE, bms.limits.ccv_max_limit);
    TEST_ASSERT_EQUAL_UINT16(CCV_MIN_LIM_FAILSAFE, bms.limits.ccv_min_limit);
    TEST_ASSERT_FALSE(bms.cl_override.bit.failsafe);

    bms_set_voltage_failsafe(false);
    TEST_ASSERT_FALSE(bms.voltage_failsafe);
    TEST_ASSERT_EQUAL_UINT16(CCV_MAX_LIM_NORMAL, bms.limits.ccv_max_limit);
    TEST_ASSERT_EQUAL_UINT16(CCV_MIN_LIM_NORMAL, bms.limits.ccv_min_limit);
    TEST_ASSERT_FALSE(bms.cl_override.bit.failsafe);
}

void test_bms_set_both_failsafe(void)
{
    bms_set_i_failsafe(true);
    bms_set_voltage_failsafe(true);
    TEST_ASSERT_TRUE(bms.i_failsafe);
    TEST_ASSERT_TRUE(bms.voltage_failsafe);

    TEST_ASSERT_EQUAL_UINT16(CCV_MAX_LIM_FAILSAFE, bms.limits.ccv_max_limit);
    TEST_ASSERT_EQUAL_UINT16(CCV_MIN_LIM_FAILSAFE, bms.limits.ccv_min_limit);
    TEST_ASSERT_TRUE(bms.cl_override.bit.failsafe);

    bms_set_i_failsafe(false);
    TEST_ASSERT_FALSE(bms.i_failsafe);
    TEST_ASSERT_TRUE(bms.voltage_failsafe);

    TEST_ASSERT_EQUAL_UINT16(CCV_MAX_LIM_FAILSAFE, bms.limits.ccv_max_limit);
    TEST_ASSERT_EQUAL_UINT16(CCV_MIN_LIM_FAILSAFE, bms.limits.ccv_min_limit);
    TEST_ASSERT_FALSE(bms.cl_override.bit.failsafe);
}
