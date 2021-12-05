/**
 * @brief SDC Interface
 *
 * This file includes functions to control the BMS's relay in the SDC..
 */

#ifndef INC_AIR_CONTROL_H_
#define INC_AIR_CONTROL_H_

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------

/**
 * Initialize PB1 as a digital output to control the transistor that controls
 * the the BMS's powerstage in the SDC.
 */
void sdc_control_init(void);

/**
 * Open the BMS powerstage in the SDC.
 */
void sdc_control_open(void);

/**
 * Close the BMS powerstage in the SDC.
 */
void sdc_control_close(void);


#endif /* INC_AIR_CONTROL_H_ */
