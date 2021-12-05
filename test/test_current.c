/**
 * @brief Test temperature.c
 *
 * This file is part of a makefile project and is intended to be built on Linux.
 */


#include "unity.h"
#include "current.h"
#include "mock/Mockdtc.h"
#include "mock/Mocksdc_control.h"

#include "bms.h"
#include "charger.h"

extern uint16_t ccl_over_sum;
extern uint16_t dcl_over_sum;
extern bool ccl_dtc_active;
extern bool dcl_dtc_active;
extern bool powerstage_fail_active;
extern uint16_t open_timer;

const float SOC_TOLERANCE = 0.05; //!< [fractional]
const float CAPACITY_TOLERANCE = 0.05 * PACK_NOMINAL_CAPACITY; //!< [A*ms]
const float SOC_LOOKUP_TOLERANCE = 0.01; //!< [fractional]
const uint16_t CURRENT_LIMIT_TOLERANCE = 5; //!< [Amps]

bms_t bms;
charger_t charger;

void setUp(void)
{
    bms_bms_init(&bms);
    bms_set_driving_limits(&(bms.limits));

    bms.powerstage_open = false;
}

void tearDown(void)
{

}

//*****************************************************************************
// Helper Functions
//*****************************************************************************

/**
 * Call once to deal with filter on ccl
 */
void helper_i_update_ccl(void)
{
    uint32_t i;
    for (i = 0; i < 100; i++)
    {
        i_update_ccl();
    }
}

/**
 * Call once to deal with filter on dcl
 */
void helper_i_update_dcl(void)
{
    uint32_t i;
    for (i = 0; i < 100; i++)
    {
        i_update_dcl();
    }
}

//*****************************************************************************
// i_amphour_integration
//*****************************************************************************

void test_i_amphour_integration_TestFullDrawAt32A(void)
{
    uint32_t i;
    uint32_t five_ms_in_hour = 100 * 2 * 60 * 60;

    float soc = 1;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = 32;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    for (i = 0; i < five_ms_in_hour; i++)
    {
        i_amphour_integration(5, &(soc), &capacity);
    }

    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 0, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, 0, capacity);
}

void test_i_amphour_integration_TestHalfDrawAt32A(void)
{
    uint32_t i;
    uint32_t five_ms_in_hour = 100 * 2 * 60 * 60;

    float soc = 1;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = 32;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    for (i = 0; i < five_ms_in_hour/2; i++)
    {
        i_amphour_integration(5, &(soc), &capacity);
    }

    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 0.5, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY * 0.5, capacity);
}

void test_i_amphour_integration_TestBeyondFullDraw(void)
{
    uint32_t i;
    uint32_t five_ms_in_hour = 100 * 2 * 60 * 60;

    float soc = 1;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = 32;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    for (i = 0; i < 2*five_ms_in_hour; i++)
    {
        i_amphour_integration(5, &(soc), &capacity);
        //printf("SOC: %f\n", soc);
    }

    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 0, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, 0, capacity);
}

void test_i_amphour_integration_TestFullChargeAt32A1SPeriod(void)
{
    uint32_t i;

    float soc = 0.0;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = -32;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    for (i = 0; i < 60; i++)
    {
        i_amphour_integration(60000, &(soc), &capacity);
    }

    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);
}

void test_i_amphour_integration_TestBeyondFullCharge(void)
{
    uint32_t i;

    float soc = 0.0;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = -32;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    for (i = 0; i < 60*2; i++)
    {
        i_amphour_integration(60000, &(soc), &capacity);
    }

    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);
}

void test_i_amphour_integration_TestHalfChargeAt32A1SPeriod(void)
{
    uint32_t i;

    float soc = 0.0;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = -32;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    for (i = 0; i < 60/2; i++)
    {
        i_amphour_integration(60000, &(soc), &capacity);
    }

    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 0.5, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY * 0.5, capacity);
}

void test_i_amphour_integration_TestOcvHighLimit(void)
{
    float soc = 0.0;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = 0;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    i_amphour_integration(60000, &(soc), &capacity);

    soc = 0.0;
    capacity = soc * PACK_NOMINAL_CAPACITY;
    i_amphour_integration(60000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 0, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, 0, capacity);

    bms.ocv_stats.average = 43000;
    i_amphour_integration(60000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);
}

void test_i_amphour_integration_TestOcvLowLimit(void)
{
    float soc = 1.0;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    bms.i_pack = 0;
    bms.ocv_stats.min = 40000;
    bms.ocv_stats.max = 40000;

    i_amphour_integration(60000, &(soc), &capacity);

    soc = 1.0;
    capacity = soc * PACK_NOMINAL_CAPACITY;
    i_amphour_integration(60000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);

    bms.ocv_stats.min = 19000;
    i_amphour_integration(60000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_TOLERANCE, 0, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, 0, capacity);
}

//*****************************************************************************
// i_lookup_soc
//*****************************************************************************
void test_i_update_soc_TestVariousValues(void)
{
    float soc = 1.0;
    float capacity = soc * PACK_NOMINAL_CAPACITY;
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);

    i_lookup_soc(30000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 0.0, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, 0, capacity);

    i_lookup_soc(31000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 0.0072, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY * 0.0072, capacity);

    i_lookup_soc(35000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 0.2243, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY * 0.2243, capacity);

    i_lookup_soc(41000, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 0.9253, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY * 0.9253, capacity);

    i_lookup_soc(41800, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);

    i_lookup_soc(42500, &(soc), &capacity);
    TEST_ASSERT_FLOAT_WITHIN(SOC_LOOKUP_TOLERANCE, 1, soc);
    TEST_ASSERT_UINT_WITHIN(CAPACITY_TOLERANCE, PACK_NOMINAL_CAPACITY, capacity);
}

//*****************************************************************************
// i_calc_first_order_ccl
//*****************************************************************************

/**
 * @test
 * Test the first order charge current limit.
 */
void test_i_calc_first_order_ccl_TestRandomValues(void)
{
    TEST_IGNORE();
    
    bms.soc = 0.6;
    bms.cr_ch_stats.average = 22;
    TEST_ASSERT_EQUAL_UINT16(182, i_calc_first_order_ccl());

    bms.soc = 0.8;
    bms.cr_ch_stats.average = 22;
    TEST_ASSERT_EQUAL_UINT16(91, i_calc_first_order_ccl());

    bms.soc = 0.9;
    bms.cr_ch_stats.average = 34;
    TEST_ASSERT_EQUAL_UINT16(47, i_calc_first_order_ccl());
}

//*****************************************************************************
// i_calc_cell_temperature_ccl
//*****************************************************************************

/**
 * @test
 * Test the cell temperature charge current limit while charging.
 */
void test_i_calc_cell_temperature_ccl_TestRandomValuesCharging(void)
{
    bms_set_charging_limits(&(bms.limits));
    bms.charger_active = true;

    bms.ct_stats.max = 20;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_cell_temperature_ccl());

    bms.ct_stats.max = 54;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_cell_temperature_ccl());

    bms.ct_stats.max = 57;
    TEST_ASSERT_EQUAL_UINT16(36, i_calc_cell_temperature_ccl());

    bms.ct_stats.max = 60;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_cell_temperature_ccl());
}

/**
 * @test
 * Test the cell temperature charge current limit while driving.
 */
void test_i_calc_cell_temperature_ccl_TestRandomValuesDriving(void)
{
    bms.ct_stats.max = 20;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_cell_temperature_ccl());

    bms.ct_stats.max = 54;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_cell_temperature_ccl());

    bms.ct_stats.max = 57;
    TEST_ASSERT_EQUAL_UINT16(36, i_calc_cell_temperature_ccl());

    bms.ct_stats.max = 60;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_cell_temperature_ccl());
}

//*****************************************************************************
// i_calc_ocv_ccl
//*****************************************************************************

/**
 * @test
 * Test the open cell voltage charge current limit while charging. Open cell
 * voltage does not affect the limit while charging.
 */
void test_i_calc_ocv_ccl_TestRandomValuesCharging(void)
{
    bms_set_charging_limits(&(bms.limits));
    bms.charger_active = true;

    bms.soc = 0.045;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_open_cell_voltage_ccl());

    bms.soc = 0.7525;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_open_cell_voltage_ccl());

    bms.soc = 1;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_open_cell_voltage_ccl());
}

/**
 * @test
 * Test the open cell voltage charge current limit while driving.
 */
void test_i_calc_ocv_ccl_TestRandomValuesDriving(void)
{
    bms.soc = 0.045;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_open_cell_voltage_ccl());

    bms.soc = 0.75;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_open_cell_voltage_ccl());

    bms.soc = 0.89;
    TEST_ASSERT_EQUAL_UINT16(60, i_calc_open_cell_voltage_ccl());

    bms.soc = 0.90;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_open_cell_voltage_ccl());

    bms.ocv_stats.average = 1;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_open_cell_voltage_ccl());
}

//*****************************************************************************
// i_update_ccl
//*****************************************************************************

/**
 * @test
 * Test the default current limit.
 */
void test_i_update_ccl_TestMaxLimit(void)
{
    bms.cl_override.all = 0;

    // the following three values need to be coordinated
    bms.soc = 0.75;
    bms.cr_ch_stats.average = 23;
    bms.ct_stats.average = 30;

    i_update_ccl();

    TEST_ASSERT_EQUAL_UINT16(60, bms.ccl);
}

/**
 * @test
 * Test when the first order limit is the lowest.
 */
void test_i_update_ccl_TestFirstOrderLimitCharging(void)
{
    TEST_IGNORE();
    
    bms_set_charging_limits(&(bms.limits));
    bms.charger_active = true;

    bms.soc = 0.90;
    bms.ct_stats.average = 25;
    bms.cr_ch_stats.average = 34;
    i_update_ccl();
    TEST_ASSERT_EQUAL_UINT16(47, bms.ccl);

    bms.soc = 0.95;
    bms.ct_stats.average = 45;
    bms.cr_ch_stats.average = 45;
    i_update_ccl();
    TEST_ASSERT_EQUAL_UINT16(30, bms.ccl);
}

/**
 * @test
 * Test when the first order limit is the lowest.
 */
void test_i_update_ccl_TestFirstOrderLimitDriving(void)
{
    TEST_IGNORE();
    
    bms.soc = 0.80;
    bms.ct_stats.average = 10;
    bms.cr_ch_stats.average = 36;
    i_update_ccl();
    TEST_ASSERT_EQUAL_UINT16(55, bms.ccl);
}

/**
 * @test
 * Test when the cell temperature is the lowest while charging.
 */
void test_i_update_ccl_TestTemperatureLimitCharging(void)
{
    bms_set_charging_limits(&(bms.limits));
    bms.charger_active = true;

    bms.soc = 0.5;
    bms.ct_stats.max = 55;
    bms.cr_ch_stats.average = 21;
    helper_i_update_ccl();
    TEST_ASSERT_EQUAL_UINT16(60, bms.ccl);

    bms.soc = 0.5;
    bms.ct_stats.max = 59;
    bms.cr_ch_stats.average = 21;
    helper_i_update_ccl();
    TEST_ASSERT_EQUAL_UINT16(12, bms.ccl);
}

/**
 * @test
 * Test when the cell temperature limit is lowest while driving.
 */
void test_i_update_ccl_TestTemperatureLimitDriving(void)
{
    bms.soc = 0.5;
    bms.ct_stats.max = 55;
    bms.cr_ch_stats.average = 21;
    helper_i_update_ccl();
    TEST_ASSERT_UINT16_WITHIN(CURRENT_LIMIT_TOLERANCE, 60, bms.ccl);

    bms.soc = 0.5;
    bms.ct_stats.max = 59;
    bms.cr_ch_stats.average = 21;
    helper_i_update_ccl();
    TEST_ASSERT_UINT16_WITHIN(CURRENT_LIMIT_TOLERANCE, 12, bms.ccl);
}

/**
 * @test
 * Test when Ocv limits is the lowest while charging.
 */
void test_i_update_ccl_TestOcvLimitCharging(void)
{
    TEST_IGNORE();
    // OCV limit not enforced while charging
}

/**
 * @test
 * Test when OCV limits is the lowest while driving.
 */
void test_i_update_ccl_TestOcvLimitDriving(void)
{
    bms.soc = 0.905;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_open_cell_voltage_ccl());
}

//*****************************************************************************
// i_calc_first_order_dcl
//*****************************************************************************

/**
 * @test
 * Test the first order calculation for discharge current limit.
 */
void test_i_calc_first_order_dcl_Test(void)
{
    bms.soc = 0.05;
    bms.cr_dch_stats.average = 29;
    TEST_ASSERT_EQUAL_UINT16(245, i_calc_first_order_dcl());

    bms.soc = 0.1;
    bms.cr_dch_stats.average = 23;
    TEST_ASSERT_EQUAL_UINT16(356, i_calc_first_order_dcl());

    bms.soc = 0.6;
    bms.cr_dch_stats.average = 21;
    TEST_ASSERT_EQUAL_UINT16(642, i_calc_first_order_dcl());
}

//*****************************************************************************
// i_calc_cell_temperature_dcl
//*****************************************************************************

/**
 * @test
 * Test the discharge current limit based on cell temperature
 */
void test_i_calc_cell_temperature_dcl_Test(void)
{
    bms.ct_stats.max = 10;
    TEST_ASSERT_EQUAL_UINT16(400, i_calc_cell_temperature_dcl());

    bms.ct_stats.max = 54;
    TEST_ASSERT_EQUAL_UINT16(400, i_calc_cell_temperature_dcl());

    bms.ct_stats.max = 57;
    TEST_ASSERT_EQUAL_UINT16(240, i_calc_cell_temperature_dcl());

    bms.ct_stats.max = 60;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_cell_temperature_dcl());
}

//*****************************************************************************
// i_calc_open_cell_voltage_dcl
//*****************************************************************************

/**
 * @test
 * Test the discharge current limit due to open cell voltage.
 */
void test_i_calc_open_cell_voltage_dcl_Test(void)
{
    bms.soc = 0;
    TEST_ASSERT_EQUAL_UINT16(0, i_calc_open_cell_voltage_dcl());

    bms.soc = 0.0225;
    TEST_ASSERT_EQUAL_UINT16(199, i_calc_open_cell_voltage_dcl());

    bms.soc = 0.045;
    TEST_ASSERT_EQUAL_UINT16(399, i_calc_open_cell_voltage_dcl());

    bms.soc = 0.90;
    TEST_ASSERT_EQUAL_UINT16(400, i_calc_open_cell_voltage_dcl());

    bms.soc = 1;
    TEST_ASSERT_EQUAL_UINT16(400, i_calc_open_cell_voltage_dcl());
}

//*****************************************************************************
// i_update_dcl
//*****************************************************************************

/**
 * @test
 * Test when the default limit.
 */
void test_i_update_dcl_TestMaxLimit(void)
{
    bms.soc = 0.9;
    bms.ct_stats.average = 25;
    bms.cr_dch_stats.average = 26;
    helper_i_update_dcl();
    TEST_ASSERT_EQUAL_UINT16(400, bms.dcl);

    bms.soc = 0.4;
    bms.ct_stats.average = 45;
    bms.cr_dch_stats.average = 21;
    helper_i_update_dcl();
    TEST_ASSERT_EQUAL_UINT16(400, bms.dcl);
}

/**
 * @test
 * Test when the first order limit is the lowest.
 */
void test_i_update_dcl_TestFirstOrderLimit(void)
{
    bms.soc = 0.1;
    bms.ct_stats.max = 25;
    bms.cr_dch_stats.average = 27;
    helper_i_update_dcl();
    TEST_ASSERT_UINT16_WITHIN(CURRENT_LIMIT_TOLERANCE, 303, bms.dcl);
}

/**
 * @test
 * Test when the OCV limit is the lowest.
 */
void test_i_update_dcl_TestOcvLimit(void)
{
    bms.soc = 0;
    bms.ct_stats.max = 45;
    bms.cr_dch_stats.average = 26;
    helper_i_update_dcl();
    TEST_ASSERT_UINT16_WITHIN(CURRENT_LIMIT_TOLERANCE, 0, bms.dcl);
}

/**
 * @test
 * Test when the cell temperature limit is the lowest.
 */
void test_i_update_dcl_TestTemperatureLimit(void)
{
    bms.soc = 0.7;
    bms.ct_stats.max = 57;
    bms.cr_dch_stats.average = 22;
    helper_i_update_dcl();
    TEST_ASSERT_UINT16_WITHIN(CURRENT_LIMIT_TOLERANCE, 240, bms.dcl);

    bms.soc = 0.7;
    bms.ct_stats.max = 60;
    bms.cr_dch_stats.average = 22;
    helper_i_update_dcl();
    TEST_ASSERT_UINT16_WITHIN(CURRENT_LIMIT_TOLERANCE, 0, bms.dcl);
}

//*****************************************************************************
// i_enforce_limits
//*****************************************************************************

/**
 * Helper function to clear variables needed for testing i_enforce_limits
 */
void crrent_enforce_clear(void)
{
    ccl_over_sum = 0;
    dcl_over_sum = 0;
    ccl_dtc_active = false;
    dcl_dtc_active = false;
    powerstage_fail_active = false;
    open_timer = 0;
}

void test_i_enforce_limits_TestCclUnder(void)
{
    uint8_t i;
    crrent_enforce_clear();

    dtc_get_update_hp_IgnoreAndReturn(false);
    dtc_send_general_Ignore(); // Makes it easier to find bugs, look for powerstage variable.

    // CCL - under limit
    bms.i_pack = -40;
    bms.ccl = 50;
    for (i = 0; i < 30; i++)
    {
        i_enforce_limits();
        if (bms.powerstage_open)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(30, i);
}

void test_i_enforce_limits_TestDclUnder(void)
{
    TEST_IGNORE();
    
    uint8_t i;
    crrent_enforce_clear();
    dtc_get_update_hp_IgnoreAndReturn(false);
    // DCL - under limit
    bms.i_pack = 40;
    bms.dcl = 50;
    for (i = 0; i < 30; i++)
    {
        i_enforce_limits();
        if (bms.powerstage_open)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(30, i);
}

void test_i_enforce_limits_TestDclUnderLeak(void)
{
    uint8_t i;
    dtc_get_update_hp_IgnoreAndReturn(false);
    crrent_enforce_clear();
    // DCL - leak
    bms.i_pack = 52;
    bms.dcl = 50;
    for (i = 0; i < 9; i++)
    {
        i_enforce_limits();
        if (bms.powerstage_open)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(9, i);

    bms.i_pack = 30;
    for (i = 0; i < 9; i++)
    {
        i_enforce_limits();
        if (bms.powerstage_open)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(9, i);

    bms.i_pack = 52;
    for (i = 0; i < 5; i++)
    {
        i_enforce_limits();
        if (bms.powerstage_open)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(5, i);
}

void test_i_enforce_limits_TestCclUnderLeak(void)
{
    uint8_t i;
    dtc_get_update_hp_IgnoreAndReturn(false);
    crrent_enforce_clear();
    bms.i_pack = -52;
    bms.ccl = 50;
    for (i = 0; i < 9; i++)
    {
       i_enforce_limits();
       if (bms.powerstage_open)
       {
           break;
       }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(9, i);

    bms.i_pack = -30;
    for (i = 0; i < 9; i++)
    {
       i_enforce_limits();
       if (bms.powerstage_open)
       {
           break;
       }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(9, i);
    
    bms.i_pack = -52;
    for (i = 0; i < 5; i++)
    {
       i_enforce_limits();
       if (bms.powerstage_open)
       {
           break;
       }
    }
    TEST_ASSERT_FALSE(bms.powerstage_open);
    TEST_ASSERT_EQUAL_UINT8(5, i);
}

void test_i_enforce_limits_TestDclOver(void)
{
    uint8_t i;
    crrent_enforce_clear();
    dtc_get_update_hp_IgnoreAndReturn(false);
    dtc_send_general_Ignore(); // Makes it easier to find bugs, look for powerstage variable.

    // DCL - over limit
    bms.i_pack = 57;
    bms.ccl = 60;
    bms.dcl = 50;
    //dtc_send_general_Expect(CL_DCL_OVER_DTC, DTC_ACTIVE);

    sdc_control_open_Expect();
    for (i = 0; i < 30; i++)
    {
        i_enforce_limits();
        if (dcl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_TRUE(dcl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(2, i);
}

void test_i_enforce_limits_TestPowerstageFail(void)
{
    crrent_enforce_clear();
    dtc_get_update_hp_IgnoreAndReturn(false);
    // Powerstage fail
    bms.powerstage_open = true;
    bms.i_pack = 20;
    sdc_control_open_Expect();
    dtc_send_general_Expect(CL_SDC_FAIL_DTC, DTC_ACTIVE);
    dtc_send_general_Expect(CL_DCL_OVER_DTC, DTC_ACTIVE);

    uint16_t cnt;
    for (cnt = 0; cnt < 2000; cnt++)
    {
        i_enforce_limits();
    }
}

void test_i_enforce_limits_TestDclOverLeak(void)
{
    uint8_t i;
    dtc_get_update_hp_IgnoreAndReturn(false);
    crrent_enforce_clear();
    // DCL - leak

    bms.i_pack = 52;
    bms.dcl = 50;

    for (i = 0; i < 9; i++)
    {
        i_enforce_limits();
        if (dcl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(dcl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(9, i);

    bms.i_pack = 30;
    for (i = 0; i < 5; i++)
    {
        i_enforce_limits();
        if (dcl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(dcl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(5, i);
    
    bms.i_pack = 57;
    dtc_send_general_Expect(CL_DCL_OVER_DTC, DTC_ACTIVE);
    sdc_control_open_Expect();
    for (i = 0; i < 9; i++)
    {
        i_enforce_limits();
        if (dcl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_TRUE(dcl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(2, i);
}

void test_i_enforce_limits_TestCclOver(void)
{
    dtc_get_update_hp_IgnoreAndReturn(false);
    dtc_send_general_Ignore(); // Makes it easier to find bugs, look for powerstage variable.
    crrent_enforce_clear();
    uint8_t i;

    // CCL - over limit
    bms.i_pack = -57;
    bms.ccl = 50;
    //dtc_send_general_Expect(CL_CCL_OVER_DTC, DTC_ACTIVE);

    sdc_control_open_Expect();
    for (i = 0; i < 30; i++)
    {
        i_enforce_limits();
        if (ccl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_TRUE(ccl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(2, i);
}

void test_i_enforce_limits_TestCclZero(void)
{
    dtc_get_update_hp_IgnoreAndReturn(false);
    crrent_enforce_clear();
    // All 0, open AIRs
    bms.i_pack = 2;
    bms.ccl = 0;
    bms.dcl = 0;

    sdc_control_open_Expect();
    i_enforce_limits();
}

void test_i_enforce_limits_TestCclOverLeak(void)
{
    uint8_t i;
    dtc_get_update_hp_IgnoreAndReturn(false);
    crrent_enforce_clear();
    // CCL - leak
    bms.i_pack = -52;
    bms.ccl = 50;

    for (i = 0; i < 9; i++)
    {
        i_enforce_limits();
        if (ccl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(ccl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(9, i);

    bms.i_pack = -30;
    for (i = 0; i < 5; i++)
    {
        i_enforce_limits();
        if (bms.powerstage_open)
        {
            break;
        }
    }
    TEST_ASSERT_FALSE(ccl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(5, i);

    bms.i_pack = -57;
    dtc_send_general_Expect(CL_CCL_OVER_DTC, DTC_ACTIVE);
    sdc_control_open_Expect();
    for (i = 0; i < 9; i++)
    {
        i_enforce_limits();
        if (ccl_dtc_active)
        {
            break;
        }
    }
    TEST_ASSERT_TRUE(ccl_dtc_active);
    TEST_ASSERT_EQUAL_UINT8(2, i);
}
