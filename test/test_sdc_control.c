/**
 * @brief Test sdc_control.c
 *
 * @par This file is part of a makefile project and is intended to be built on
 * Linux.
 */

#include "unity.h"
#include "mock/Mockdtc.h"
#include "sdc_control.h"

#include "bms.h"
#include "charger.h"

bms_t bms;
charger_t charger;

void setUp(void)
{
    bms_bms_init(&bms);
    charger_t_init(&charger);
}

void tearDown(void)
{

}

void test_sdc_control_open(void)
{
    // powerstage should default close
    TEST_ASSERT_TRUE(bms.powerstage_open);
    TEST_ASSERT_FALSE(charger.charge_enable);

    // test closing the powerstage
    dtc_send_general_Expect(0x0605, DTC_ACTIVE);
    sdc_control_close();
    TEST_ASSERT_FALSE(bms.powerstage_open);

    // test opening the powerstage
    dtc_send_general_Expect(0x0604, DTC_ACTIVE);
    sdc_control_open();
    TEST_ASSERT_TRUE(bms.powerstage_open);

    // TODO further test the charger
}
