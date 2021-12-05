/**
 * @brief Test temperature.c
 *
 * @par This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include "unity.h"
#include "temperature.h"

#include "cell_asic_mutators.h"
#include "mock/Mockdtc.h"

#include "lookup.h"
#include "bms.h"
#include "ltc6811.h"
#include "params.h"

bms_t bms;
cell_asic bms_ic[TOTAL_IC];

extern uint16_t bsbt_active_fault;
extern uint16_t bsbh_active_fault;
extern uint16_t ct_active_fault;
extern bool ct_lost_active;

void reset_temperature_globl_vars(void)
{
    bsbt_active_fault = BSBTH_DTC;
    bsbh_active_fault = BSBTH_DTC;
    ct_active_fault = CT_DTC;
    ct_lost_active = false;
}

void setUp(void)
{
    dtc_get_update_lp_IgnoreAndReturn(false);
    dtc_get_update_hp_IgnoreAndReturn(false);
    bms_bms_init(&bms);
    bms_set_driving_limits(&(bms.limits));
    reset_temperature_globl_vars();
}

void tearDown(void)
{

}

int8_t adc_to_temp(uint16_t adc)
{
    float voltage = adc / 10000.0; // LSB = 100uV
    return lut_get_1d(&CT_LUT, voltage) + 0.5;
}

int8_t bsbt_ratio_to_temp(uint16_t ratio)
{
    return -66.875 + (218.75 * ratio) / 99;
}

uint8_t bsbh_ratio_to_humidity(uint16_t ratio)
{
    return -25 + (250 * ratio) / 99;
}

void ct_test_fault_all(uint16_t temp, uint16_t new_dtc, uint16_t last_dtc)
{
    ct_lost_active = false;
    bms.ct_stats.num_invalid_sensors = 0;

    uint8_t cell;
    cam_set_all_cell_temperatures(bms_ic, temp);


    int8_t temperature = adc_to_temp(temp);

    // Expect calls need to be in order for each function
    for (cell = 0; cell < TOTAL_CELLS; cell++)
    {
        bms.ct_stats.fault_codes[cell] = last_dtc; // Setup struct with last fault, as it gets cleared each test
        if (new_dtc == CT_OORL_DTC || new_dtc == CT_OORH_DTC)
        {
            dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, cell, temp);
            temperature = new_dtc == CT_OORL_DTC ? 0x7F : 0x80;
        }
        else if (new_dtc != CT_DTC)
        {
           dtc_send_cell_8bit_Expect(new_dtc, DTC_ACTIVE, cell, temperature);
        }

        if (last_dtc == CT_OORL_DTC || last_dtc == CT_OORH_DTC)
        {
            dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, cell, temp);
        }
        else if (last_dtc != CT_DTC)
        {
           dtc_send_cell_8bit_Expect(last_dtc, DTC_INACTIVE, cell, temperature);
        }
    }
    
    if (new_dtc == CT_OORL_DTC || new_dtc == CT_OORH_DTC)
    {
        // At least 5 sensors lost
        dtc_send_general_Expect(CT_LOST_SENSE_DTC, DTC_ACTIVE);
    }

    tmprtr_stats_ct();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.ct_stats.fault_codes[4]); // check arbitrary cell
    if (new_dtc == CT_OORL_DTC || new_dtc == CT_OORH_DTC)
    {
        TEST_ASSERT_TRUE(bms.cl_override.bit.cell_lost_sensors);
    }

}

void ct_test_fault_cell(uint8_t cell, uint16_t cell_temp, uint16_t rest_temp, uint16_t new_dtc, uint16_t last_dtc)
{
    ct_lost_active = false;
    bms.ct_stats.num_invalid_sensors = 0;

    cam_set_all_cell_temperatures(bms_ic, rest_temp);
    cam_set_cell_temperature(bms_ic, cell_temp, cell);

    int8_t temperature = adc_to_temp(cell_temp);

    bms.ct_stats.fault_codes[cell] = last_dtc; // Setup struct with last fault, as it gets cleared each test

    if (new_dtc == CT_OORL_DTC || new_dtc == CT_OORH_DTC)
    {
        dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, cell, cell_temp);
        temperature = new_dtc == CT_OORL_DTC ? 0x7F : 0x80;
    }
    else if (new_dtc != CT_DTC)
    {
        dtc_send_cell_8bit_Expect(new_dtc, DTC_ACTIVE, cell, temperature);
    }

    if (last_dtc == CT_OORL_DTC || last_dtc == CT_OORH_DTC)
    {
        bms.ct_stats.num_invalid_sensors = 1; // if OOR, already one invalid sensor
        dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, cell, cell_temp);
    }
    else if (last_dtc != CT_DTC)
    {
        dtc_send_cell_8bit_Expect(last_dtc, DTC_INACTIVE, cell, temperature);
    }

    tmprtr_stats_ct();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.ct_stats.fault_codes[cell]);
    bms.ct_stats.fault_codes[cell] = CT_DTC; // reset fault codes
}

void bsbt_test_fault_all(uint16_t ratio, uint16_t new_dtc, uint16_t last_dtc)
{
    bms.bsbt_stats.num_invalid_sensors = 0;

    uint8_t bsb;
    cam_set_all_bsbt_ratios(bms_ic, ratio);

    int8_t temperature = bsbt_ratio_to_temp(ratio);

    // Expect calls need to be in order for each function
    for (bsb = 0; bsb < TOTAL_BSB; bsb++)
    {
        bms.bsbt_stats.fault_codes[bsb] = last_dtc; // Setup struct with last fault, as it gets cleared each test
        if (new_dtc == BSBT_OORL_DTC || new_dtc == BSBT_OORH_DTC)
        {
            dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, bsb, ratio);
            temperature = new_dtc == BSBT_OORH_DTC ? 0x7F : 0x80;

        }
        else if (new_dtc != BSBTH_DTC)
        {
           dtc_send_cell_8bit_Expect(new_dtc, DTC_ACTIVE, bsb, temperature);
        }

        if (last_dtc == BSBT_OORL_DTC || last_dtc == BSBT_OORH_DTC)
        {
            dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, bsb, ratio);
            bms.bsbt_stats.num_invalid_sensors++;
        }
        else if (last_dtc != BSBTH_DTC)
        {
           dtc_send_cell_8bit_Expect(last_dtc, DTC_INACTIVE, bsb, temperature);
        }
    }
    
    tmprtr_stats_bsbt();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.bsbt_stats.fault_codes[3]); // check arbitrary bsb
}

void bsbh_test_fault_all(uint16_t ratio, uint16_t new_dtc, uint16_t last_dtc)
{
    bms.bsbh_stats.num_invalid_sensors = 0;

    uint8_t bsb;
    cam_set_all_bsbh_ratios(bms_ic, ratio);

    uint8_t humidity = bsbh_ratio_to_humidity(ratio);

    // Expect calls need to be in order for each function
    for (bsb = 0; bsb < TOTAL_BSB; bsb++)
    {
        bms.bsbh_stats.fault_codes[bsb] = last_dtc; // Setup struct with last fault, as it gets cleared each test
        if (new_dtc == BSBH_OORL_DTC || new_dtc == BSBH_OORH_DTC)
        {
            dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, bsb, ratio);
            humidity = new_dtc == BSBH_OORH_DTC ? 0x7F : 0x80;
        }
        else if (new_dtc != BSBTH_DTC)
        {
           dtc_send_cell_8bit_Expect(new_dtc, DTC_ACTIVE, bsb, humidity);
        }

        if (last_dtc == BSBH_OORL_DTC || last_dtc == BSBH_OORH_DTC)
        {
            dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, bsb, ratio);
            bms.bsbh_stats.num_invalid_sensors++;
        }
        else if (last_dtc != BSBTH_DTC)
        {
           dtc_send_cell_8bit_Expect(last_dtc, DTC_INACTIVE, bsb, humidity);
        }
    }
    
    tmprtr_stats_bsbh();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.bsbh_stats.fault_codes[3]); // check arbitrary bsb
}

void bsbh_test_fault_bsb(uint8_t bsb, uint16_t cell_ratio, uint16_t rest_ratio, uint16_t new_dtc, uint16_t last_dtc)
{
    bms.bsbh_stats.num_invalid_sensors = 0;

    cam_set_all_bsbh_ratios(bms_ic, rest_ratio);
    cam_set_bsbh_ratio(bms_ic, cell_ratio, bsb);

    int8_t humidity = bsbh_ratio_to_humidity(cell_ratio);


    bms.bsbh_stats.fault_codes[bsb] = last_dtc; // Setup struct with last fault, as it gets cleared each test

    if (new_dtc == BSBH_OORL_DTC || new_dtc == BSBH_OORH_DTC)
    {
        dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, bsb, cell_ratio);
        humidity = new_dtc == BSBH_OORH_DTC ? 0x7F : 0x80;
    }
    else if (new_dtc != BSBTH_DTC)
    {
        dtc_send_cell_8bit_Expect(new_dtc, DTC_ACTIVE, bsb, humidity);
    }

    if (last_dtc == BSBH_OORL_DTC || last_dtc == BSBH_OORH_DTC)
    {
        bms.bsbh_stats.num_invalid_sensors = 1; // if OOR, already one invalid sensor
        dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, bsb, cell_ratio);
    }
    else if (last_dtc != BSBTH_DTC)
    {
        dtc_send_cell_8bit_Expect(last_dtc, DTC_INACTIVE, bsb, humidity);
    }

    tmprtr_stats_bsbh();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.bsbh_stats.fault_codes[bsb]);
    bms.bsbh_stats.fault_codes[bsb] = BSBTH_DTC; // reset fault codes
}

void bsbt_test_fault_bsb(uint8_t bsb, uint16_t cell_ratio, uint16_t rest_ratio, uint16_t new_dtc, uint16_t last_dtc)
{
    bms.bsbt_stats.num_invalid_sensors = 0;

    cam_set_all_bsbt_ratios(bms_ic, rest_ratio);
    cam_set_bsbt_ratio(bms_ic, cell_ratio, bsb);

    int8_t temperature = bsbt_ratio_to_temp(cell_ratio);


    bms.bsbt_stats.fault_codes[bsb] = last_dtc; // Setup struct with last fault, as it gets cleared each test

    if (new_dtc == BSBT_OORL_DTC || new_dtc == BSBT_OORH_DTC)
    {
        dtc_send_cell_16bit_Expect(new_dtc, DTC_ACTIVE, bsb, cell_ratio);
        temperature = new_dtc == BSBT_OORH_DTC ? 0x7F : 0x80;
    }
    else if (new_dtc != BSBTH_DTC)
    {
        dtc_send_cell_8bit_Expect(new_dtc, DTC_ACTIVE, bsb, temperature);
    }

    if (last_dtc == BSBT_OORL_DTC || last_dtc == BSBT_OORH_DTC)
    {
        bms.bsbt_stats.num_invalid_sensors = 1; // if OOR, already one invalid sensor
        dtc_send_cell_16bit_Expect(last_dtc, DTC_INACTIVE, bsb, cell_ratio);
    }
    else if (last_dtc != BSBTH_DTC)
    {
        dtc_send_cell_8bit_Expect(last_dtc, DTC_INACTIVE, bsb, temperature);
    }

    tmprtr_stats_bsbt();

    TEST_ASSERT_EQUAL_UINT(new_dtc, bms.bsbt_stats.fault_codes[bsb]);
    bms.bsbt_stats.fault_codes[bsb] = BSBTH_DTC; // reset fault codes
}

//*****************************************************************************
// tmprtr_stats_ct
//*****************************************************************************

/**
 * @test
 * Calculate temperature statistics when all cells are OORL.
 */
void test_tmprtr_stats_ct_TestAllOorl(void)
{
    cam_set_all_cell_temperatures(bms_ic, 12900);

    dtc_send_general_Expect(CT_LOST_SENSE_DTC, DTC_ACTIVE);
    dtc_send_cell_16bit_Ignore(); // ignore the OORL fault for each cell
    tmprtr_stats_ct();


    TEST_ASSERT_EQUAL_UINT(60, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(0, bms.ct_stats.average);

    // None valid, no results
    TEST_ASSERT_EQUAL_INT(0, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT((int32_t)0xffffffffffffffff, bms.ct_stats.min);
}

/**
 * @test
 * Calculate temperature statistics when one cell is OORL.
 */
void test_tmprtr_stats_ct_TestOneCellOorl(void)
{
    cam_set_all_cell_temperatures(bms_ic, 17500);
    cam_set_cell_temperature(bms_ic, 12900, 0);

    dtc_send_cell_16bit_Ignore(); // ignore the OORL fault for each cell
    tmprtr_stats_ct();

    TEST_ASSERT_EQUAL_UINT8(1, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT8(34, bms.ct_stats.average);
    TEST_ASSERT_EQUAL_INT8(34, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT8(34, bms.ct_stats.min);
}

/**
 * @test
 * Calculate cell temperature statistics when all cells are OORH.
 */
void test_tmprtr_stats_ct_TestAllOorh(void)
{
    cam_set_all_cell_temperatures(bms_ic, 24450);

    dtc_send_general_Expect(CT_LOST_SENSE_DTC, DTC_ACTIVE);
    dtc_send_cell_16bit_Ignore(); // ignore the OORL fault for each cell
    tmprtr_stats_ct();


    TEST_ASSERT_EQUAL_UINT(60, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(0, bms.ct_stats.average);

    // None valid, no results
    TEST_ASSERT_EQUAL_INT(0, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT((int32_t)0xffffffffffffffff, bms.ct_stats.min);
}

/**
 * @test
 * Calculate cell temperature statistics when one cell is OORH.
 */
void test_tmprtr_stats_ct_TestOneCellOorh(void)
{
    cam_set_all_cell_temperatures(bms_ic, 17500);
    cam_set_cell_temperature(bms_ic, 24450, 0);

    dtc_send_cell_16bit_Ignore(); // ignore the OORL fault for each cell
    tmprtr_stats_ct();

    TEST_ASSERT_EQUAL_UINT8(1, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT8(34, bms.ct_stats.average);
    TEST_ASSERT_EQUAL_INT8(34, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT8(34, bms.ct_stats.min);
}

void test_temperature_stats_cell_temperature_UNDER_all(void)
{
    ct_test_fault_all(23750, CT_UNDER_DTC, CT_DTC);
    ct_test_fault_all(24000, CT_UNDER_DTC, CT_OVER_DTC);

    int8_t temp = adc_to_temp(24000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.average);
    
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.min);
    TEST_ASSERT_TRUE(bms.cl_override.bit.cell_temp);
}

void test_temperature_stats_cell_temperature_UNDER_ind(void)
{
    ct_test_fault_cell(3, 24399, 18500, CT_UNDER_DTC, CT_DTC);
    ct_test_fault_cell(32, 24050, 19000, CT_UNDER_DTC, CT_OORL_DTC);

    int8_t cell_temp = adc_to_temp(24050);
    int8_t rest_temp = adc_to_temp(19000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 59 + cell_temp) / 60, bms.ct_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(cell_temp, bms.ct_stats.min);
    TEST_ASSERT_TRUE(bms.cl_override.bit.cell_temp);
}

void test_temperature_stats_cell_temperature_LOW_all(void)
{
    ct_test_fault_all(22900, CT_LOW_DTC, CT_DTC); // 22851
    ct_test_fault_all(23000, CT_LOW_DTC, CT_OVER_DTC);

    int8_t temp = adc_to_temp(23000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.min);
}

void test_temperature_stats_cell_temperature_LOW_ind(void)
{
    ct_test_fault_cell(4, 22900, 18500, CT_LOW_DTC, CT_DTC);
    ct_test_fault_cell(55, 23500, 19000, CT_LOW_DTC, CT_OORL_DTC);

    int8_t cell_temp = adc_to_temp(23500);
    int8_t rest_temp = adc_to_temp(19000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 59 + cell_temp) / 60, bms.ct_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(cell_temp, bms.ct_stats.min);
}

void test_temperature_stats_cell_temperature_OVER_all(void)
{
    ct_test_fault_all(15000, CT_OVER_DTC, CT_DTC);
    ct_test_fault_all(14000, CT_OVER_DTC, CT_HIGH_DTC);

    int8_t temp = adc_to_temp(14000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.min);
    TEST_ASSERT_TRUE(bms.cl_override.bit.cell_temp);
}

void test_temperature_stats_cell_temperature_OVER_ind(void)
{
    ct_test_fault_cell(5, 13001, 18500, CT_OVER_DTC, CT_DTC);
    ct_test_fault_cell(17, 15020, 19000, CT_OVER_DTC, CT_OORL_DTC);

    int8_t cell_temp = adc_to_temp(15020);
    int8_t rest_temp = adc_to_temp(19000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 59 + cell_temp) / 60, bms.ct_stats.average);

    TEST_ASSERT_EQUAL_INT(cell_temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.ct_stats.min);
    TEST_ASSERT_TRUE(bms.cl_override.bit.cell_temp);
}

void test_temperature_stats_cell_temperature_HIGH_all(void)
{
    ct_test_fault_all(15750, CT_HIGH_DTC, CT_DTC);
    ct_test_fault_all(15600, CT_HIGH_DTC, CT_OVER_DTC);

    int8_t temp = adc_to_temp(15600);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.min);
}

void test_temperature_stats_cell_temperature_HIGH_ind(void)
{
    ct_test_fault_cell(6, 15727, 18500, CT_HIGH_DTC, CT_DTC);
    ct_test_fault_cell(31, 15555, 19000, CT_HIGH_DTC, CT_OORL_DTC);

    int8_t cell_temp = adc_to_temp(15555);
    int8_t rest_temp = adc_to_temp(19000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 59 + cell_temp) / 60, bms.ct_stats.average);

    TEST_ASSERT_EQUAL_INT(cell_temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.ct_stats.min);
}

void test_temperature_stats_cell_temperature_NORMAL_all(void)
{
    ct_test_fault_all(16000, CT_DTC, CT_DTC);
    ct_test_fault_all(18500, CT_DTC, CT_OVER_DTC);

    int8_t temp = adc_to_temp(18500);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.ct_stats.min);
}

void test_temperature_stats_cell_temperature_NORMAL_ind(void)
{
    ct_test_fault_cell(7, 17000, 18500, CT_DTC, CT_DTC);
    ct_test_fault_cell(23, 21000, 19000, CT_DTC, CT_OORL_DTC);

    int8_t cell_temp = adc_to_temp(21000);
    int8_t rest_temp = adc_to_temp(19000);

    TEST_ASSERT_EQUAL_UINT(0, bms.ct_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 59 + cell_temp) / 60, bms.ct_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.ct_stats.max);
    TEST_ASSERT_EQUAL_INT(cell_temp, bms.ct_stats.min);
}

//*****************************************************************************
// tmprtr_stats_bsbt
//*****************************************************************************

void test_temperature_stats_bsbt_OORL_all(void)
{
    bsbt_test_fault_all(9, BSBT_OORL_DTC, BSBTH_DTC);
    bsbt_test_fault_all(1, BSBT_OORL_DTC, BSBT_LOW_DTC);

    TEST_ASSERT_EQUAL_UINT(6, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(0, bms.bsbt_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_INT(-128, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(127, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_OORL_ind(void)
{
    bsbt_test_fault_bsb(0, 2, 40, BSBT_OORL_DTC, BSBTH_DTC);
    bsbt_test_fault_bsb(5, 7, 50, BSBT_OORL_DTC, BSBT_HIGH_DTC);

    int8_t rest_temp = bsbt_ratio_to_temp(50);

    TEST_ASSERT_EQUAL_UINT(1, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_OORH_all(void)
{
    bsbt_test_fault_all(91, BSBT_OORH_DTC, BSBTH_DTC);
    bsbt_test_fault_all(99, BSBT_OORH_DTC, BSBT_OORL_DTC);

    TEST_ASSERT_EQUAL_UINT(6, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(0, bms.bsbt_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_INT(-128, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(127, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_OORH_ind(void)
{
    bsbt_test_fault_bsb(1, 100, 40, BSBT_OORH_DTC, BSBTH_DTC);
    bsbt_test_fault_bsb(4, 94, 50, BSBT_OORH_DTC, BSBT_LOW_DTC);

    int8_t rest_temp = bsbt_ratio_to_temp(50);

    TEST_ASSERT_EQUAL_UINT(1, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_LOW_all(void)
{
    bsbt_test_fault_all(15, BSBT_LOW_DTC, BSBTH_DTC);
    bsbt_test_fault_all(19, BSBT_LOW_DTC, BSBT_OORH_DTC);

    int8_t temp = bsbt_ratio_to_temp(19);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT8(temp, bms.bsbt_stats.average);
    TEST_ASSERT_EQUAL_INT8(temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT8(temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_LOW_ind(void)
{
    bsbt_test_fault_bsb(2, 12, 40, BSBT_LOW_DTC, BSBTH_DTC);
    bsbt_test_fault_bsb(3, 20, 50, BSBT_LOW_DTC, BSBT_HIGH_DTC);

    int8_t rest_temp = bsbt_ratio_to_temp(50);
    int8_t bsb_temp = bsbt_ratio_to_temp(20);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 5 + bsb_temp) / 6, bms.bsbt_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(bsb_temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_HIGH_all(void)
{
    bsbt_test_fault_all(67, BSBT_HIGH_DTC, BSBTH_DTC);
    bsbt_test_fault_all(80, BSBT_HIGH_DTC, BSBT_LOW_DTC);

    int8_t temp = bsbt_ratio_to_temp(80);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.bsbt_stats.average);

    TEST_ASSERT_EQUAL_INT(temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_HIGH_ind(void)
{
    bsbt_test_fault_bsb(3, 58, 40, BSBT_HIGH_DTC, BSBTH_DTC);
    bsbt_test_fault_bsb(2, 70, 50, BSBT_HIGH_DTC, BSBT_LOW_DTC);

    int8_t rest_temp = bsbt_ratio_to_temp(50);
    int8_t bsb_temp = bsbt_ratio_to_temp(70);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 5 + bsb_temp) / 6, bms.bsbt_stats.average);

    TEST_ASSERT_EQUAL_INT(bsb_temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_NORMAL_all(void)
{
    bsbt_test_fault_all(22, BSBTH_DTC, BSBTH_DTC);
    bsbt_test_fault_all(44, BSBTH_DTC, BSBT_LOW_DTC);

    int8_t temp = bsbt_ratio_to_temp(44);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(temp, bms.bsbt_stats.average);
    TEST_ASSERT_EQUAL_INT(temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(temp, bms.bsbt_stats.min);
}

void test_temperature_stats_bsbt_NORMAL_ind(void)
{
    bsbt_test_fault_bsb(2, 24, 40, BSBTH_DTC, BSBTH_DTC);
    bsbt_test_fault_bsb(3, 46, 50, BSBTH_DTC, BSBT_HIGH_DTC);

    int8_t rest_temp = bsbt_ratio_to_temp(50);
    int8_t bsb_temp = bsbt_ratio_to_temp(46);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbt_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT((rest_temp * 5 + bsb_temp) / 6, bms.bsbt_stats.average);

    TEST_ASSERT_EQUAL_INT(rest_temp, bms.bsbt_stats.max);
    TEST_ASSERT_EQUAL_INT(bsb_temp, bms.bsbt_stats.min);
}

//*****************************************************************************
// tmprtr_broadcast_ct
//*****************************************************************************

void test_temperature_broadcast_ct_TestOorh(void)
{
    uint8_t cell_id = 0;
    uint8_t bsb_id;
    uint8_t c_i;
    bms_cell_to_bsb_id_ct(cell_id, &bsb_id, &c_i);

    bms.ct_stats.average = 10;

    bms_ic[bsb_id].cells.c_codes[c_i] = 25000;
    dtc_send_cell_16bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(10, tmprtr_calc_ct(cell_id));

    bms_ic[bsb_id].cells.c_codes[c_i] = 30000;
    dtc_send_cell_16bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(10, tmprtr_calc_ct(cell_id));
}

void test_temperature_broadcast_ct_TestOorl(void)
{
    dtc_send_16bit_Ignore();

    uint8_t cell_id = 0;
    uint8_t bsb_id;
    uint8_t c_i;
    bms_cell_to_bsb_id_ct(cell_id, &bsb_id, &c_i);

    bms.ct_stats.average = 10;

    bms_ic[bsb_id].cells.c_codes[c_i] = 12900;
    dtc_send_cell_16bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(10, tmprtr_calc_ct(cell_id));

    bms_ic[bsb_id].cells.c_codes[c_i] = 500;
    dtc_send_cell_16bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(10, tmprtr_calc_ct(cell_id));
}

void test_temperature_broadcast_ct_TestValidValues(void)
{
    uint8_t cell_id = 0;
    uint8_t bsb_id;
    uint8_t c_i;
    bms_cell_to_bsb_id_ct(cell_id, &bsb_id, &c_i);

    bms_ic[bsb_id].cells.c_codes[c_i] = 13500;
    dtc_send_cell_8bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(95, tmprtr_calc_ct(cell_id));

    bms_ic[bsb_id].cells.c_codes[c_i] = 17500;
    dtc_send_cell_8bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(cell_id));

    bms_ic[bsb_id].cells.c_codes[c_i] = 20000;
    dtc_send_cell_8bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(14, tmprtr_calc_ct(cell_id));

    bms_ic[bsb_id].cells.c_codes[c_i] = 23500;
    dtc_send_cell_8bit_Ignore();
    TEST_ASSERT_EQUAL_INT8(-19, tmprtr_calc_ct(cell_id));
}

//*****************************************************************************
// tmprtr_stats_bsbh
//*****************************************************************************

void test_temperature_stats_bsbh_OORL_all(void)
{
    bsbh_test_fault_all(9, BSBH_OORL_DTC, BSBTH_DTC);
    bsbh_test_fault_all(1, BSBH_OORL_DTC, BSBH_LOW_DTC);

    TEST_ASSERT_EQUAL_UINT(6, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(0, bms.bsbh_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(127, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_OORL_ind(void)
{
    bsbh_test_fault_bsb(0, 2, 40, BSBH_OORL_DTC, BSBTH_DTC);
    bsbh_test_fault_bsb(5, 7, 30, BSBH_OORL_DTC, BSBH_HIGH_DTC);

    uint8_t rest_humidity = bsbh_ratio_to_humidity(30);

    TEST_ASSERT_EQUAL_UINT(1, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_INT(rest_humidity, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_OORH_all(void)
{
    bsbh_test_fault_all(91, BSBH_OORH_DTC, BSBTH_DTC);
    bsbh_test_fault_all(99, BSBH_OORH_DTC, BSBH_OORL_DTC);

    TEST_ASSERT_EQUAL_UINT(6, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.average);
    // None valid, no results
    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(127, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_OORH_ind(void)
{
    bsbh_test_fault_bsb(1, 100, 40, BSBH_OORH_DTC, BSBTH_DTC);
    bsbh_test_fault_bsb(4, 94, 30, BSBH_OORH_DTC, BSBH_LOW_DTC);

    uint8_t rest_humidity = bsbh_ratio_to_humidity(30);

    TEST_ASSERT_EQUAL_UINT(1, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_LOW_all(void)
{
    bsbh_test_fault_all(15, BSBH_LOW_DTC, BSBTH_DTC);
    bsbh_test_fault_all(25, BSBH_LOW_DTC, BSBH_HIGH_DTC);

    uint8_t humidity = bsbh_ratio_to_humidity(25);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.average);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_LOW_ind(void)
{
    bsbh_test_fault_bsb(2, 12, 40, BSBH_LOW_DTC, BSBTH_DTC);
    bsbh_test_fault_bsb(3, 20, 30, BSBH_LOW_DTC, BSBH_HIGH_DTC);

    uint8_t rest_humidity = bsbh_ratio_to_humidity(30);
    uint8_t bsb_humidity = bsbh_ratio_to_humidity(20);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT((rest_humidity * 5 + bsb_humidity) / 6, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(bsb_humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_HIGH_all(void)
{
    bsbh_test_fault_all(50, BSBH_HIGH_DTC, BSBTH_DTC);
    bsbh_test_fault_all(56, BSBH_HIGH_DTC, BSBH_LOW_DTC);

    uint8_t humidity = bsbh_ratio_to_humidity(56);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_HIGH_ind(void)
{
    bsbh_test_fault_bsb(3, 58, 40, BSBH_HIGH_DTC, BSBTH_DTC);
    bsbh_test_fault_bsb(2, 56, 30, BSBH_HIGH_DTC, BSBH_LOW_DTC);

    uint8_t rest_humidity = bsbh_ratio_to_humidity(30);
    uint8_t bsb_humidity = bsbh_ratio_to_humidity(56);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT((rest_humidity * 5 + bsb_humidity) / 6, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(bsb_humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_WARN_all(void)
{
    bsbh_test_fault_all(61, BSBH_WARN_DTC, BSBTH_DTC);
    bsbh_test_fault_all(80, BSBH_WARN_DTC, BSBH_LOW_DTC);

    uint8_t humidity = bsbh_ratio_to_humidity(80);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(CHAR_MAX, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_WARN_ind(void)
{
    bsbh_test_fault_bsb(3, 65, 40, BSBH_WARN_DTC, BSBTH_DTC);
    bsbh_test_fault_bsb(2, 75, 30, BSBH_WARN_DTC, BSBH_HIGH_DTC);

    uint8_t rest_humidity = bsbh_ratio_to_humidity(30);
    uint8_t bsb_humidity = bsbh_ratio_to_humidity(75);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT((rest_humidity * 5 + bsb_humidity) / 6, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(bsb_humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_NORMAL_all(void)
{
    bsbh_test_fault_all(30, BSBTH_DTC, BSBTH_DTC);
    bsbh_test_fault_all(49, BSBTH_DTC, BSBH_LOW_DTC);

    uint8_t humidity = bsbh_ratio_to_humidity(49);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(humidity, bms.bsbh_stats.min);
}

void test_temperature_stats_bsbh_NORMAL_ind(void)
{
    bsbh_test_fault_bsb(3, 35, 40, BSBTH_DTC, BSBTH_DTC);
    bsbh_test_fault_bsb(2, 44, 30, BSBTH_DTC, BSBH_HIGH_DTC);

    uint8_t rest_humidity = bsbh_ratio_to_humidity(30);
    uint8_t bsb_humidity = bsbh_ratio_to_humidity(44);

    TEST_ASSERT_EQUAL_UINT(0, bms.bsbh_stats.num_invalid_sensors);
    TEST_ASSERT_EQUAL_UINT((rest_humidity * 5 + bsb_humidity) / 6, bms.bsbh_stats.average);

    TEST_ASSERT_EQUAL_UINT(bsb_humidity, bms.bsbh_stats.max);
    TEST_ASSERT_EQUAL_UINT(rest_humidity, bms.bsbh_stats.min);
}

//*****************************************************************************
//
//*****************************************************************************

void test_temperature_stats_internal(void)
{
    int8_t temperature = 0;
    // OORL
    dtc_send_16bit_Expect(IT_OORL_DTC, DTC_ACTIVE, 1000);
    tmprtr_stats_internal(1000);
    TEST_ASSERT_EQUAL_INT(0, bms.int_mcu_temp);

    // OORH
    dtc_send_16bit_Expect(IT_OORH_DTC, DTC_ACTIVE, 3200);
    dtc_send_16bit_Expect(IT_OORL_DTC, DTC_INACTIVE, 3200);
    tmprtr_stats_internal(3200);
    TEST_ASSERT_EQUAL_INT(0, bms.int_mcu_temp);

    // HIGH
    temperature = 147.5 - ((75.0 * 3.3 * 1257) / 4095);
    dtc_send_8bit_Expect(IT_HIGH_DTC, DTC_ACTIVE, temperature);
    dtc_send_16bit_Expect(IT_OORH_DTC, DTC_INACTIVE, 1257);
    tmprtr_stats_internal(1257);
    TEST_ASSERT_EQUAL_INT(temperature, bms.int_mcu_temp);

    // LOW
    temperature = 147.5 - ((75.0 * 3.3 * 2797) / 4095.0);
    dtc_send_8bit_Expect(IT_LOW_DTC, DTC_ACTIVE, temperature);
    dtc_send_8bit_Expect(IT_HIGH_DTC, DTC_INACTIVE, temperature);
    tmprtr_stats_internal(2797);
    TEST_ASSERT_EQUAL_INT(temperature, bms.int_mcu_temp);

    // NORMAL
    temperature = 147.5 - ((75.0 * 3.3 * 1500) / 4095);
    dtc_send_8bit_Expect(IT_LOW_DTC, DTC_INACTIVE, temperature);
    tmprtr_stats_internal(1500);
    TEST_ASSERT_EQUAL_INT(temperature, bms.int_mcu_temp);
} 

//*****************************************************************************
// tmprtr_broadcast_bsbth
//*****************************************************************************

void test_temperature_broadcast_bsbth_Test(void)
{
    TEST_IGNORE();
}

//*****************************************************************************
// tmprtr_calc_ct
//*****************************************************************************

/**
 * @test
 * Test the conversion from ADC value to cell voltage for a non-fault
 * temperature value.
 */
void test_tmprtr_calc_ct_TestNormalRange(void)
{
    cam_set_cell_temperature(bms_ic, 17500, 21);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));

    cam_set_cell_temperature(bms_ic, 20000, 21);
    TEST_ASSERT_EQUAL_INT8(14, tmprtr_calc_ct(21));
}

/**
 * @test
 * Test that an OORL temperature voltage gets set and cleared.
 */
void test_tmprtr_calc_ct_TestOorl(void)
{
    // activate fault
    cam_set_cell_temperature(bms_ic, 12900, 21);
    dtc_send_cell_16bit_Expect(CT_OORL_DTC, DTC_ACTIVE, 21, 12900);
    TEST_ASSERT_EQUAL_INT8(0, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_OORL_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(1, bms.ct_stats.num_invalid_sensors);

    // clear fault
    cam_set_cell_temperature(bms_ic, 17500, 21);
    dtc_send_cell_16bit_Expect(CT_OORL_DTC, DTC_INACTIVE, 21, 17500);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);
}

/**
 * @test
 * Test that an OORH temperature voltage gets set and cleared.
 */
void test_tmprtr_calc_ct_TestOorh(void)
{
    // activate fault
    cam_set_cell_temperature(bms_ic, 24450, 21);
    dtc_send_cell_16bit_Expect(CT_OORH_DTC, DTC_ACTIVE, 21, 24450);
    TEST_ASSERT_EQUAL_INT8(0, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_OORH_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(1, bms.ct_stats.num_invalid_sensors);

    // clear fault
    cam_set_cell_temperature(bms_ic, 17500, 21);
    dtc_send_cell_16bit_Expect(CT_OORH_DTC, DTC_INACTIVE, 21, 17500);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);
}

/**
 * @test
 * Test that a high temperature fault is set and cleared.
 */
void test_tmprtr_calc_ct_TestHighLim(void)
{
    // activate fault
    cam_set_cell_temperature(bms_ic, 15500, 21);
    dtc_send_cell_8bit_Expect(CT_HIGH_DTC, DTC_ACTIVE, 21, 55);
    TEST_ASSERT_EQUAL_INT8(55, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_HIGH_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);

    // clear fault
    cam_set_cell_temperature(bms_ic, 17500, 21);
    dtc_send_cell_8bit_Expect(CT_HIGH_DTC, DTC_INACTIVE, 21, 34);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);
}

/**
 * @test
 * Test that a max temperature fault is set and cleared.
 */
void test_tmprtr_calc_ct_TestMaxLim(void)
{
    // activate fault
    cam_set_cell_temperature(bms_ic, 15000, 21);
    dtc_send_cell_8bit_Expect(CT_OVER_DTC, DTC_ACTIVE, 21, 62);
    TEST_ASSERT_EQUAL_INT8(62, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_OVER_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);

    // clear fault
    cam_set_cell_temperature(bms_ic, 17500, 21);
    dtc_send_cell_8bit_Expect(CT_OVER_DTC, DTC_INACTIVE, 21, 34);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);
}

/**
 * @test
 * Test that a low temperature fault is set and cleared.
 */
void test_tmprtr_calc_ct_TestLowLim(void)
{
    // activate fault
    cam_set_cell_temperature(bms_ic, 23000, 21);
    dtc_send_cell_8bit_Expect(CT_LOW_DTC, DTC_ACTIVE, 21, -12);
    TEST_ASSERT_EQUAL_INT8(-12, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_LOW_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);

    // clear fault
    cam_set_cell_temperature(bms_ic, 17500, 21);
    dtc_send_cell_8bit_Expect(CT_LOW_DTC, DTC_INACTIVE, 21, 34);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);
}

/**
 * @test
 * Test that a min temperature fault is set and cleared.
 */
void test_tmprtr_calc_ct_TestMinLim(void)
{
    // activate fault
    cam_set_cell_temperature(bms_ic, 23600, 21);
    dtc_send_cell_8bit_Expect(CT_UNDER_DTC, DTC_ACTIVE, 21, -21);
    TEST_ASSERT_EQUAL_INT8(-21, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_UNDER_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);

    // clear fault
    cam_set_cell_temperature(bms_ic, 17500, 21);
    dtc_send_cell_8bit_Expect(CT_UNDER_DTC, DTC_INACTIVE, 21, 34);
    TEST_ASSERT_EQUAL_INT8(34, tmprtr_calc_ct(21));
    TEST_ASSERT_EQUAL_UINT16(CT_DTC, bms.ct_stats.fault_codes[21]);
    TEST_ASSERT_EQUAL_UINT8(0, bms.ct_stats.num_invalid_sensors);
}
