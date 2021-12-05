/**
 * @brief BMS Application Interface
 *
 * This file includes functions to interact with the BMS Application
 *
 * This file is part of a CCS project and intended to be built with CCS
 *
 */

#ifndef INC_BMS_H_
#define INC_BMS_H_

#include <stdbool.h>
#include <stdint.h>
#include "eeprom.h"

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------
#define IC_PER_BSB 1
#define CELLS_PER_BSB 11

#ifdef DEBUG  // defined under "Predefinied Symbols"
#define TOTAL_BSBS 1
#else
#define TOTAL_BSBS 7
#endif

#define TOTAL_ICS (TOTAL_BSBS * IC_PER_BSB)
#define TOTAL_CELLS (TOTAL_BSBS * CELLS_PER_BSB)
#define PACK_NOMINAL_CAPACITY (32.0f * 3600 * 1000)  // [Ah] * [3600 s / hr] * [1000 ms / s] = [A*ms]

typedef struct
{
    int32_t max;
    uint32_t max_id;
    int32_t min;
    uint32_t min_id;
    int32_t average;
    int32_t invalid_count;
} stats_t;


typedef struct
{
    uint16_t closed_cell_voltage;  //!< [0.1 mV]
    uint16_t open_cell_voltage;  //!< [0.1 mV]
    uint8_t ch_i_limit;  //!< [Amps]
    uint16_t dch_i_limit;  //!< [Amps]

    uint8_t charge_res;  //!< [0.1 mOhm]
    uint8_t discharge_res;  //!< [0.1 mOhm]
    int8_t temperature;  //!< [deg C]

    int32_t capacity;  //!< [A*ms]
    int32_t energy;  //!< [Watt*sec]
    float soc;  //!< [Fraction]
    float soe;  //!< [Fraction]

    bool balancing_active;
} cell_t;


typedef struct
{
    uint16_t closed_cell_voltage_max; //!< Limit for overvoltage fault [V]
    uint16_t closed_cell_voltage_min; //!< Limit for undervoltage fault [V]
} limits_t;


typedef struct
{
    // Pack current information
    float i_pack[5];  //!< Pack Current [1 Amps] Discharge Current is Positive
    float i_pack_offset;  //!< Drift Offset for Current [1 Amp]
    float i_pack_voltage;  //!< [Amps]

    uint16_t discharge_current_limit;  //!< Discharge Current Limit [1 Amp]
    uint16_t charge_current_limit;  //!< Charge Current Limit [1 Amp]

    //-------------------------------------------------------------------------
    // Statistics
    //-------------------------------------------------------------------------
    stats_t discharge_res_stats;  //!< Cell Discharge Resistance Stats
    stats_t charge_res_stats;  //!< Cell Charge Resistance Stats
    stats_t open_cell_voltage_stats;  //!< Open Cell Voltage Stats
    stats_t closed_cell_voltage_stats;  //!< Closed Cell Voltage Stats
    stats_t cell_temperature_stats; //!< Cell temperature stats
    stats_t cell_soc_stats;  //!< State of charge stats
    stats_t cell_capacity_stats;  //!< Cell capacity stats
    stats_t cell_ch_i_limit_stats;  //!< Cell charge current limit stats
    stats_t cell_dch_i_limit_stats;  //!< Cell discharge current limit stats

    // CAN Rx Values
    bool airs_closed;  // received on CAN
    bool cell_balancing_commanded;  // recieved on CAN

    // EEPROM information
    eeprom_stats stored_stats;          //!< Statistics that are tracked in eeprom
    uint16_t eeprom_write_ctr;          //!< Counter for number of time writing to eeprom.

    // Balancing Info
    uint8_t balancing_cell_id;          //!< Cell ID of the cell that's actively balancing
    uint8_t num_cells_balancing;        //!< Number of cells with DCC switches enabled.
    uint16_t balancing_vltg_limit;      //!< Voltage limit to discharge to for cell balancing

    limits_t limits;                    //!< Safety Limits

    cell_t cells[TOTAL_CELLS];  //!< Cell information
    uint16_t pack_ccv;  //!< Pack closed cell voltage [0.01 Volts]
    uint16_t pack_ocv;  //!< Pack open cell voltage [0.01 Volts]
    uint16_t pack_charge_res;  //!< Pack charge resistance [mOhms]
    uint16_t pack_discharge_res;  //!< PAck discharge resistance [mOhms]
} bms_t;

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------


extern bms_t bms;

/**
 * Initialize the BMS struct.
 */
void bms_init(void);


uint8_t bms_bsb_to_cell_id(uint8_t ic_id, uint8_t c_i);

/**
 * Convert Cell ID (CT) to BSB IC ID and IC C_i
 *
 * @param cell_id to convert
 * @param ic_id pointer to store the calculate IC ID
 * @param c_i pointer to store the calculated IC C Index
 */
void bms_cell_to_bsb_id_ct(uint8_t cell_id, uint8_t *ic_id, uint8_t *c_i);

/**
 * Convert Cell ID (CCV) to BSB IC ID and IC C_i
 *
 * @param cell_id to convert
 * @param ic_id pointer to store the calculate IC ID
 * @param c_i pointer to store the calculated IC C Index
 */
void bms_cell_to_bsb_id_ccv(uint8_t cell_id, uint8_t *ic_id, uint8_t *c_i);

/**
 * Convert a Channel ID and a BSB ID to two cell IDs
 *
 * @param ch The channel to convert
 * @param ic_id The BSB IC the cell is on
 * @param cell_array Array of length 2 to store cell ids in
 */
void bms_ch_to_cell_ids(uint8_t ch, uint8_t ic_id, uint8_t cell_array[]);
void bms_cr_calc(void);

/**
 * @brief Initialize CR based on SOC and CT
 *
 * Look up cell charge and discharge resistance based on SOC and cell
 * temperature. This function can be called to set the initial value of the
 * cell resistances.
 */
void bms_lookup_cell_res(void);

/**
 * @brief Set the state of the current (I) failsafe.
 *
 * @param active True if failsafe should be set to active.
 */
void bms_set_i_failsafe(bool active);

/**
 * @brief Set the state of the Voltage (V) failsafe.
 *
 * @param active True if failsafe should be set to active.
 */
void bms_set_voltage_failsafe(bool active);

/**
 * @brief Updates the idle state of the car/battery pack.
 *
 * Tracks time low current to determine if pack has been idle
 * long enough
 *
 * @param update_period The period the function is called at. [ms]
 */
void bms_update_idle_state(uint16_t update_period);

/**
 * @brief set limits for charging
 *
 * @param l Pointer to limits variable
 */
void bms_set_charging_limits(limits_t *l);

/**
 * @brief set limits for driving
 *
 * @param l Pointer to limits variable
 */
void bms_set_driving_limits(limits_t *l);

/**
 * @brief Calculate minimum, maximum, and average.
 *
 * Cell Discharge Resistance, Cell Charge Resistance, Cell Temperature,
 * Closed Cell Voltage, and Open Cell Voltage.
 */
void bms_calc_cell_stats(void);

/**
 * @brief Transfer Cell Temperature From IO Layer to Application
 *
 * For the given @param channel take the measurements
 * from the bms ic struct and place into the bms struct.
 */
void bms_transfer_cell_temperature(uint8_t channel);


/**
 * @brief Transfer Cell Voltages From IO Layer to Application
 *
 * For the given @param channel, take the measurements from the @param bms_ic
 * struct and place into the bms struct.
 */
void bms_transfer_cell_voltages(uint8_t channel);

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------


/**
 * @return Pack current draw [Amps]
 */
float bms_get_i_pack(void);


/**
 * @return Pack current offset [Amps]
 */
float bms_get_i_pack_offset(void);


/**
 * @return Pack CCV [Amps]
 */
float bms_get_i_pack_voltage(void);


/**
 * @return measured closed cell voltage [0.1 mV]
 */
uint16_t bms_get_closed_cell_voltage(uint8_t cell_i);


/**
 * If the voltage measurement is OOR use the average cell voltage for
 * calculations.
 *
 * @return closed cell voltage to be used for calculations [0.1 mV]
 */
uint16_t bms_get_closed_cell_voltage_calc(uint8_t cell_i);


/**
 * @return open cell voltage [0.1 mV]
 */
uint16_t bms_get_open_cell_voltage(uint8_t cell_i);


/**
 * If the voltage measurement is OOR then use the average cell voltage for
 * calculations.
 *
 * @return open cell voltage to be used for calculations [0.1 mV]
 */
uint16_t bms_get_open_cell_voltage_calc(uint8_t cell_i);


/**
 * @return cell temperature [deg C]
 */
int8_t bms_get_cell_temperature(uint8_t cell_i);


/**
 * If the temperature measurement is OOR then use the average cell
 * temperature for calculations.
 */
int8_t bms_get_cell_temperature_calc(uint8_t cell_i);


/**
 * @return cell capacity [A*ms]
 */
uint32_t bms_get_cell_capacity(uint8_t cell_i);


/**
 * @return cell state of charge [fractional]
 */
float bms_get_pack_soc(uint8_t cell_i);


/**
 * @param cell_i
 * @return state of charge [fractional]
 */
float bms_get_cell_soc(uint8_t cell_i);


/**
 * @return cell id of cell with highest state of charge
 */
uint8_t bms_get_cell_soc_max_id(void);


/**
 * @return highest cell state of charge [fractional]
 */
float bms_get_cell_soc_max(void);


/**
 * @return cell id of cell with lowest state of charge
 */
uint8_t bms_get_cell_soc_min_id(void);


/**
 * @return lowest cell state of charge [fractional]
 */
float bms_get_cell_soc_min(void);


/**
 * @return average cell state of charge [fractional]
 */
float bms_get_cell_soc_average(void);


uint8_t bms_get_cell_capacity_max_id(void);


int32_t bms_get_cell_capacity_min(void);


int32_t bms_get_cell_capacity_max(void);


uint8_t bms_get_cell_capacity_min_id(void);


int32_t bms_get_cell_capacity_average(void);

uint8_t bms_get_cell_ch_i(uint8_t cell_i);

uint16_t bms_get_cell_dch_i(uint8_t cell_i);

uint8_t bms_get_cell_ch_i_limit_max_id(void);


uint16_t bms_get_cell_ch_i_limit_max(void);


uint8_t bms_get_cell_ch_i_limit_min_id(void);


uint16_t bms_get_cell_ch_i_limit_min(void);


uint16_t bms_get_cell_ch_i_limit_average(void);


uint8_t bms_get_cell_dch_i_limit_max_id(void);


uint16_t bms_get_cell_dch_i_limit_max(void);


uint8_t bms_get_cell_dch_i_limit_min_id(void);


uint16_t bms_get_cell_dch_i_limit_min(void);


uint16_t bms_get_cell_dch_i_limit_average(void);


uint8_t bms_get_cell_charge_res(uint8_t cell_i);


uint8_t bms_get_cell_discharge_res(uint8_t cell_i);


uint8_t bms_get_cell_charge_tau(uint8_t cell_i);


uint8_t bms_get_cell_discharge_tau(uint8_t cell_i);


int32_t bms_get_cell_energy(uint8_t cell_i);


float bms_get_cell_soe(uint8_t cell_i);


bool bms_get_cell_balancing_active(uint8_t cell_i);

//-----------------------------------------------------------------------------
// Setters
//-----------------------------------------------------------------------------


/**
 * Update the cell capacity with the capacity draw.
 *
 * @param cell_i
 * @param capacity_delta [A*ms]
 */
void bms_update_cell_capacity_draw(uint8_t cell_i, int32_t capacity_delta);


/**
 * Set the capacity of the cell.
 *
 * @param cell_i
 * @param capacity [A*ms]
 */
void bms_set_cell_capacity(uint8_t cell_i, int32_t capacity);


/**
 * Update the cell state of charge with the soc draw
 *
 * @param cell_i
 * @param soc_delta
 */
void bms_update_cell_soc_draw(uint8_t cell_i, float soc_delta);


/**
 * Set the state of charge of the cell.
 *
 * @param cell_i
 * @param soc [fractional]
 */
void bms_set_cell_soc(uint8_t cell_i, float soc);


/**
 * @brief Checks each cell for voltage faults.
 */
void bms_check_for_voltage_faults(void);


#endif /* INC_BMS_H_ */
