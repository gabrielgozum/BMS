/**
 * @brief Handles lookup tables and interpolation
 *
 * Contains functions for using lookup tables to get values.
 *
 */

#include "lookup.h"

/**
 * @brief Returns the corresponding value based on the table.
 *
 * Interpolates between table values to determine output. Step size is fixed.
 *
 * @param table The lookup table to reference.
 * @param input The input value to match output with.
 * @return The interpolated value of the input.
 */
float lut_get_1d(const lut_1d_t *table, float input)
{
    // find the lower bound index
    float f_index = ((input - table->in_base) / table->in_step);
    uint16_t index = (uint16_t)f_index;

    // Clip if out of range
    // need to check with float values, as int between -1 and 0 rounds to 0.
    if (f_index < 0)
    {
        return table->out_values[0];
    }
    else if (index >= table->length - 1) // Must have at least one index above found value.
    {
        return table->out_values[table->length - 1];
    }

    float bottom_val = table->in_base + table->in_step * (float)index;

    float ratio = (input - bottom_val) / table->in_step;
    return table->out_values[index] + ratio * (table->out_values[index+1] - table->out_values[index]);
}

/**
 * @brief Returns the corresponding value based on the table, using 2 independent variables.
 *
 * Interpolates between table values to determine output. Step size is fixed.
 *
 * @param table The lookup table to reference.
 * @param input_1 The input on the first axis.
 * @param input_2 The input on the second axis.
 * @return The interpolated value based on the inputs.
 */
float lut_get_2d(const lut_2d_t *table, float input_1, float input_2)
{
    // Note: array condensed to 1 dimension
    uint16_t increment = table->length_2;

    // find the lower bound index
    float f_index_1 = ((input_1 - table->in_base_1) / table->in_step_1);
    float f_index_2 = ((input_2 - table->in_base_2) / table->in_step_2);
    uint16_t index_1 = (uint16_t)f_index_1;
    uint16_t index_2 = (uint16_t)f_index_2;

    // Clip if out of range
    // need to check with float values, as int between -1 and 0 rounds to 0.
    if (f_index_1 < 0.0)
    {
        if (f_index_2 < 0)
        {
            return table->out_values[0];
        }
        // use ints here, want rounding down.
        else if (index_2 >= table->length_2 - 1)
        {
            return table->out_values[table->length_2 - 1];
        }
        else
        {
            // still interpolate on y
            index_1 = 0;
            input_1 = table->in_base_1;
        }
    }
    else if (index_1 >= table->length_1 - 1) // Must have at least one index above found value.
    {
        if (f_index_2 < 0)
        {
            return (table->out_values + increment * (table->length_1 - 1))[0];
        }
        else if (index_2 >= table->length_2 - 1)
        {
            return (table->out_values + increment * (table->length_1 - 1))[table->length_2 - 1];
        }
        else
        {
            // still interpolate on y
            index_1 = table->length_1 - 1;
            input_1 = table->in_base_1 + (table->in_step_1 * (table->length_1-1));
        }
    }

    // don't need to check x out of bounds again. Can assume in range.
    if (f_index_2 < 0)
    {
        index_2 = 0;
        input_2 = table->in_base_2;
    }
    else if (index_2 >= table->length_2 - 1) // Must have at least one index above found value.
    {
        index_2 = table->length_2 - 1;
        input_2 = table->in_base_2 + (table->in_step_2 * (table->length_2-1));
    }

    float bottom_val_1 = table->in_base_1 + table->in_step_1  * (float)index_1;
    float ratio_1 = (input_1 - bottom_val_1) / table->in_step_1;

    float r1 = (table->out_values + increment * (index_1))[index_2] + ratio_1 * ((table->out_values + increment * (index_1+1))[index_2] - (table->out_values + increment * (index_1))[index_2]);
    float r2 = (table->out_values + increment * (index_1))[index_2+1] + ratio_1 * ((table->out_values + increment * (index_1+1))[index_2+1] - (table->out_values + increment * (index_1))[index_2+1]);

    float bottom_val_2 = table->in_base_2 + table->in_step_2  * (float)index_2;
    float ratio_2 = (input_2 - bottom_val_2) / table->in_step_2;

    return r1 + ratio_2 * (r2 - r1);
}
