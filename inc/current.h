/**
 * @brief Current sensing and limits interface.
 *
 * This file includes functions to calculate SOC, DCL, and CCL.
 *
 */

#ifndef INC_CURRENT_H_
#define INC_CURRENT_H_

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
typedef enum
{
    PACK_CALCULATION,
    CELL_CALCULATION
} calc_level_e;

// ----------------------------------------------------------------------------
// Declarations
// ----------------------------------------------------------------------------

/**
 * Calculate Maximum discharge current based on CCV, OCV, Resistance
 */
void current_update_dcl(void);

/**
 * Calculate Charge Current Limit
 */
void current_update_ccl(void);

/**
 * @brief Stops the current from exceeding the limit for too long or by too much.
 *
 * Uses a leaky bucket integrator to track time/amount above limit. Once the limit
 * is exceeded for too long, this function opens the AIRs.
 *
 * Additionally, if both dcl and i_pack are 0, the AIRs are opened, as no current should be flowing.
 */
void current_enforce_limits(void);

/**
 * @brief Amphour Integration
 *
 * Integrate the pack current for the given integration period.
 *
 * @param integration_period [ms]
 *
 * @return Amp*ms diff in capacity
 */
int32_t current_amphour_integration(float integration_period);

/**
 * @param ocv Open Circuit Cell Voltage [0.1mV]
 *
 * @return state of charge [fractional]
 */
float current_lookup_soc(uint16_t ocv);

/**
 * @brief Updates SOC
 *
 * Calculates SOC using either amphour integration or lookup.
 * Handles idle state and failsafe conditions. Do at teh pack level and cell
 * level.
 */
void current_update_soc(void);


/**
 * @brief      Get current measurement from ADC
 *
 * @return     pack current [Amps]
 */
float current_get_measurement(void);


/**
 * @brief      Calculate drift offset
 * 
 * Must be called when current is know to be zero.
 *
 * @return     current offset [Amps]
 */
float current_calc_offset(void);


/**
 * @breif Start ADC measurement for i_pack
 */
void current_start_measurement(void);


#endif /* INC_CURRENT_H_ */
