/**
 * @brief Test ltc1864.c
 *
 * This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "unity.h"

#include "bms.h"
#include "ltc1864.h"
#include "params.h"

#include "mock/Mockdtc.h"
#include "mock/Mockspi.h"

extern uint16_t i_active_dtc;

bms_t bms;


void setCSHigh(uint32_t base);
void setCSLow(uint32_t base);

void setUp(void)
{
    dtc_get_update_lp_IgnoreAndReturn(false);
    dtc_get_update_hp_IgnoreAndReturn(false);

    dtc_send_16bit_Ignore();
    bms_bms_init(&bms);
}

void tearDown(void)
{

}

void test_ltc1864_calc_offset(void)
{
    int i;

    uint32_t adc_val = 32000;

    for (i = 0; i < OFFSET_AVERAGE_SAMPLES + 1; i++)
    {
        spiTransfer_ExpectAndReturn(0xff, SSI0_BASE, (adc_val >> 8) & 0xFF);
        spiTransfer_ExpectAndReturn(0xff, SSI0_BASE, (adc_val) & 0xFF);
    }

    float voltage = adc_val * 5.0 / 65535.0;
    float i_val = -(voltage - 2.5) * 1000.0 / I_SENSE_GAIN;

    bms.int_i = i_val;
    float avg = ltc1864_calc_offset();
    TEST_ASSERT_FLOAT_WITHIN(0.1, i_val, avg);
}

void i_run_test(uint16_t adc_val, uint16_t new_dtc, uint16_t last_dtc)
{
    i_active_dtc = IS_DTC; // reset fault codes

    spiTransfer_ExpectAndReturn(0xff, SSI0_BASE, (adc_val >> 8) & 0xFF);
    spiTransfer_ExpectAndReturn(0xff, SSI0_BASE, (adc_val) & 0xFF);

    if (new_dtc != IS_DTC)
    {
        dtc_send_16bit_Expect(new_dtc, DTC_ACTIVE, adc_val);
    }
    
    if (new_dtc != IS_COMPARE_DTC)
    {
        float voltage = adc_val * 5.0 / 65535.0;
        float i_val = (voltage - 2.5) * 1000.0 / I_SENSE_GAIN;
        bms.int_i =  i_val;
    }

    if (last_dtc != IS_DTC)
    {
        i_active_dtc = last_dtc;
        dtc_send_16bit_Expect(last_dtc, DTC_INACTIVE, adc_val);
    }

    ltc1864_get_measurement();

    TEST_ASSERT_EQUAL_HEX16(new_dtc, i_active_dtc);
}   

void test_ltc1864_get_measurement_OORL(void)
{
    bms.i_pack = 0;

    i_run_test(99, IS_OORL_DTC, IS_DTC);
    i_run_test(10, IS_OORL_DTC, IS_OORH_DTC);

    TEST_ASSERT_FLOAT_WITHIN(0.1, 0, bms.i_pack);
}

void test_ltc1864_get_measurement_OORH(void)
{
    bms.i_pack = 0;

    i_run_test(65535, IS_OORH_DTC, IS_DTC);
    i_run_test(65535, IS_OORH_DTC, IS_OORL_DTC);

    TEST_ASSERT_EQUAL_UINT(0, bms.i_pack);
}

/* Disabled in BMS code
void test_ltc1864_get_measurement_DIFF(void)
{
    bms.i_pack = 0;
    bms.int_i = 50.0;

    i_run_test(50000, IS_COMPARE_DTC, IS_DTC);
    i_run_test(65000, IS_OORH_DTC, IS_OORL_DTC);

    TEST_ASSERT_FLOAT_WITHIN(0.1, 0, bms.i_pack);
}
*/

void test_ltc1864_get_measurement_NORMAL(void)
{
    bms.i_pack = 0;

    i_run_test(1312, IS_DTC, IS_DTC);
    i_run_test(64000, IS_DTC, IS_COMPARE_DTC);

    float voltage = 64000 * 5.0 / 65535.0;
    float i_val = -(voltage - 2.5) * 1000.0 / I_SENSE_GAIN;

    TEST_ASSERT_FLOAT_WITHIN(0.1, i_val, bms.i_pack);
}

