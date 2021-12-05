#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "unity.h"
#include "faults.h"


uint8_t cell_overtemperature_flags[8];
uint8_t cell_undertemperature_flags[8];
uint8_t cell_overvoltage_flags[8];
uint8_t cell_undervoltage_flags[8];
uint8_t highimpedence_flags[8];

void setUp(void)
{


}


void tearDown(void)
{


}

void test_set_fault_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        set_fault_active(i, cell_overtemperature_flags);
        TEST_ASSERT_BIT_HIGH((i % 8), cell_overtemperature_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_clear_fault(void)
{
    for(int i = 0; i < 60; i++)
    {

        clear_fault(i, cell_overtemperature_flags);
        TEST_ASSERT_BIT_LOW((i % 8), cell_overtemperature_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_is_active_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        TEST_ASSERT_FALSE(is_active_fault(i, cell_overtemperature_flags));
        set_fault_active(i, cell_overtemperature_flags);
        TEST_ASSERT_TRUE(is_active_fault(i, cell_overtemperature_flags));

    }

}

void test_set_overtemperature_active(void)
{

    for(int i = 0; i < 60; i++)
    {
        set_overtemperature_fault_active(i, 0);
        TEST_ASSERT_BIT_HIGH((i % 8), cell_overtemperature_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_clear_overtemperature_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        clear_overtemperature_fault(i);
        TEST_ASSERT_BIT_LOW((i % 8), cell_overtemperature_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_is_overtemperature_fault_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        TEST_ASSERT_FALSE(is_overtemperature_fault_active(i));
        set_overtemperature_fault_active(i,0);
        TEST_ASSERT_TRUE(is_overtemperature_fault_active(i));

    }

}

void test_set_undertemperature_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        set_undertemperature_fault_active(i, 0);
        TEST_ASSERT_BIT_HIGH((i % 8), cell_undertemperature_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_clear_undertemperature_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        clear_undertemperature_fault(i);
        TEST_ASSERT_BIT_LOW((i % 8), cell_undertemperature_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_is_undertemperature_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        TEST_ASSERT_FALSE(is_undertemperature_active_fault(i));
        set_undertemperature_fault_active(i,0);
        TEST_ASSERT_TRUE(is_undertemperature_active_fault(i));

    }

}

void test_set_overvoltage_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        set_overvoltage_fault_active(i, 0);
        TEST_ASSERT_BIT_HIGH((i % 8), cell_overvoltage_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_clear_overvoltage_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        clear_overvoltage_fault(i);
        TEST_ASSERT_BIT_LOW((i % 8), cell_overvoltage_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_is_overvoltage_active_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        TEST_ASSERT_FALSE(is_overvoltage_active_fault(i));
        set_overvoltage_fault_active(i,0);
        TEST_ASSERT_TRUE(is_overvoltage_active_fault(i));

    }

}

void test_set_undervoltage_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        set_undervoltage_fault_active(i, 0);
        TEST_ASSERT_BIT_HIGH((i % 8), cell_undervoltage_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_clear_undervoltage_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        clear_undervoltage_fault(i);
        TEST_ASSERT_BIT_LOW((i % 8), cell_undervoltage_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_is_undervoltage_active_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        TEST_ASSERT_FALSE(is_undervoltage_active_fault(i));
        set_undervoltage_fault_active(i,0);
        TEST_ASSERT_TRUE(is_undervoltage_active_fault(i));

    }

}

void test_set_highimpedence_active(void)
{

    for(int i = 0; i < 60; i++)
    {

        set_highimpedence_active(i, 0);
        TEST_ASSERT_BIT_HIGH((i % 8), highimpedence_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_clear_highimpedence_fault(void)
{

    for(int i = 0; i < 60; i++)
    {

        clear_highimpedence_fault(i);
        TEST_ASSERT_BIT_LOW((i % 8), highimpedence_flags[i/8] & (uint8_t)( pow((2.0), (double)(i % 8)) ));

    }

}

void test_is_highimpedence_active_fault(void){

    for(int i = 0; i < 60; i++)
    {

        TEST_ASSERT_FALSE(is_highimpedence_active_fault(i));
        set_highimpedence_active(i,0);
        TEST_ASSERT_TRUE(is_highimpedence_active_fault(i));

    }

}
