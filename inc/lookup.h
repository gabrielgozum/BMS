/**
 * @brief Handles lookup tables and interpolation
 *
 * Contains functions for using lookup tables to get values.
 *
 */

#ifndef INC_LOOKUP_H_
#define INC_LOOKUP_H_

#include <stdint.h>

// ----------------------------------------------------------------------------
//
// Definitions
//
// ----------------------------------------------------------------------------

/**
 * @brief Contains parameters to create a interpolated lookup
 * table with 1 independent variable, and 1 dependent variable.
 *
 * The input axis is fixed width, specified only with a base
 * and step size.
 */
typedef struct
{
    const uint32_t length; //!< The number of elements in the lookup table

    const float in_base; //!< The number the input axis starts at. Corresponds to out_values[0].
    const float in_step; //!< The spacing on the input axis between elements.

    const float *out_values; //!< The array of values on the output axis.
} lut_1d_t;

/**
 * @brief Contains parameters to create a interpolated lookup
 * table with 2 independent variables, and 1 dependent variable.
 *
 * The input axes are fixed width, specified only with a base
 * and step size.
 *
 * out_values is a 1d array, as C does not have pointers to 2d
 * arrays of arbitrary dimensions. Every <length_2> Elements is a row.
 * E.g. the first row is from out_values[0] to out_values[length_2-1],
 * the second row is from out_values[length_2] to out_values[2*length_2-1].
 *
 * out_values can be initialized to a 2d array. Cast to type (const float *) to get
 * a pointer to the first element. In memory, the structure is identical.
 * E.g:
 * @code{.cpp}
 * const float[3][3] lookup_data = {{0,1,2}, {3,4,5}, {6,7,8}};
 * const lut_2d_t lookup_table = {
 *  length_1, length_2,
 *  in_base_1, in_step_1, in_base_2, in_step_2,
 *  (const float *)lookup_data // Explicit type cast
 * };
 * @endcode
 */
typedef struct
{
    const uint32_t length_1; //!< The number of elements on the first variable. Must be this many rows in out_value.
    const uint32_t length_2; //!< The number of elements on the second variable. Must be this many columns in out_value.

    const float in_base_1; //!< The number the first input axis starts at. Specifies row.
    const float in_step_1; //!< The spacing on the first input axis between elements.

    const float in_base_2; //!< The number the second input axis starts at. Specifies column.
    const float in_step_2; //!< The spacing on the second input axis between elements.

    const float *out_values; //!< The array of values on the output axis. 1d array, treats every length_2 elements a row. @see lut_2d_t.
} lut_2d_t;

// Convenience macro for creating luts

/**
 * @brief Creates a constant 1d lookup table struct with the given name and parameters.
 *
 * @param name The name of the constant to create
 * @param length The number of elements in the lookup table
 * @param in_base The number the input axis starts at
 * @param in_step The spacing on the input axis between elements
 * @param out_array Array of output values corresponding to the input base and step
 */
#define LUT_1D_INIT(name, length, in_base, in_step, out_array) static const lut_1d_t name = {length, in_base, in_step, out_array}

/**
 * @brief Creates a constant 2d lookup table struct with the given name and parameters.
 *
 * @param name The name of the constant to create
 * @param length_1 The number of elements on the first input axis
 * @param length_2 The number of elements on the second input axis
 * @param in_base_1 The number the first input axis starts at
 * @param in_step_1 The spacing on the first input axis between elements
 * @param in_base_2 The number the second input axis starts at
 * @param in_step_2 The spacing on the second input axis between elements
 * @param out_array Array of output values corresponding to the input bases and steps. @see lut_2d_t for array format.
 */
#define LUT_2D_INIT(name, length_1, length_2, in_base_1, in_step_1, in_base_2, in_step_2, out_array) \
        static const lut_2d_t name = {length_1, length_2, in_base_1, in_step_1, in_base_2, in_step_2, (const float *)out_array}

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------
float lut_get_1d(const lut_1d_t *table, float input);
float lut_get_2d(const lut_2d_t *table, float input_1, float input_2);

#endif /* INC_LOOKUP_H_ */
