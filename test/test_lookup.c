/**
 * @file test_lookup.c
 *
 * @brief Test lookup.c
 *
 * This file is part of a makefile project and is intended to be built on Linux.
 */

#include "unity.h"
#include "lookup.h"

// max allowed error from calculated value
#define ERROR 0.05

// = -sin(0.3x)
const float out_1d[] = {-0.9969, -0.929, -0.7781, -0.5577, -0.2875, 0.0084, 0.3036, 0.5716, 0.7885, 0.9351};
// = -sin(0.3x) * 2.1cos(0.1y)
float out_2d[][10] =
{
 {1.1269, 1.0997, 1.0195, 0.8902, 0.7180, 0.5111, 0.2797, 0.3470, -0.2119, -0.4484}, // 0.5366
 {1.0731, 1.0472, 0.9709, 0.8478, 0.6838, 0.4868, 0.2663, 0.0330, -0.2018, -0.4270}, // 0.5110
 {1.0185, 0.9940, 0.9215, 0.8046, 0.6489, 0.4620, 0.2528, 0.0314, -0.1916, -0.4052}, // 0.4850
 {0.9630, 0.9399, 0.8713, 0.7608, 0.6136, 0.4368, 0.2390, 0.0297, -0.1811, -0.3832} // 0.4586
};

const lut_1d_t test_1d = { 10,
                     5.5,
                     1.0,
                     out_1d };

const lut_2d_t test_2d = { 4,
                     10,
                     40,
                     0.1,
                     0.0,
                     2.2,
                     (const float *)out_2d };

void setUp(void)
{

}

void tearDown(void)
{

}

void test_lookup_1d_out_of_range(void)
{
    float below = lut_get_1d(&test_1d, 4.5);
    float above = lut_get_1d(&test_1d, 20);

    TEST_ASSERT_EQUAL_FLOAT(-0.9969, below);
    TEST_ASSERT_EQUAL_FLOAT(0.9351, above);
}

void test_lookup_2d_out_of_range_both(void)
{
    float below_below = lut_get_2d(&test_2d, 30.0, -0.1);
    float below_above = lut_get_2d(&test_2d, 0.0, 100.0);
    float above_above = lut_get_2d(&test_2d, 50.0, 50.4);
    float above_below = lut_get_2d(&test_2d, 41.0, -55.0);


    TEST_ASSERT_EQUAL_FLOAT(1.1269, below_below);
    TEST_ASSERT_EQUAL_FLOAT(-0.4484, below_above);
    TEST_ASSERT_EQUAL_FLOAT(-0.3832, above_above);
    TEST_ASSERT_EQUAL_FLOAT(0.9630, above_below);
}

void test_lookup_2d_out_of_range_one(void)
{
    float below_in = lut_get_2d(&test_2d, 0.0, 10);
    float above_in = lut_get_2d(&test_2d, 50.0, 4.4);
    float in_above = lut_get_2d(&test_2d, 40.2, 100.0);
    float in_below = lut_get_2d(&test_2d, 40.1, -0.1);


    TEST_ASSERT_FLOAT_WITHIN(ERROR, 0.6088, below_in);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, 0.8712, above_in);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, -0.4052, in_above);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, 1.0731, in_below);
}

void test_lookup_1d_in_range(void)
{
    float edge = lut_get_1d(&test_1d, 5.5);
    float step = lut_get_1d(&test_1d, 8.5);
    float middle1 = lut_get_1d(&test_1d, 6.0);
    float middle2 = lut_get_1d(&test_1d, 7.34);

    TEST_ASSERT_FLOAT_WITHIN(ERROR, -0.9969, edge);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, -0.5577, step);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, -0.9738, middle1);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, -0.8073, middle2);
}

void test_lookup_2d_in_range(void)
{
    float edge_edge = lut_get_2d(&test_2d, 40, 0.0);
    float step_edge = lut_get_2d(&test_2d, 40.2, 19.8);
    float step_middle = lut_get_2d(&test_2d, 40.1, 11.8);

    TEST_ASSERT_FLOAT_WITHIN(ERROR, 1.1268, edge_edge);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, -0.4052, step_edge);
    TEST_ASSERT_FLOAT_WITHIN(ERROR, 0.4088, step_middle);
}
