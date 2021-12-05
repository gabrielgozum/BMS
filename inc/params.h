/*
 * @brief BMS Parameters
 *
 * Definitions for limits and configuration values.
 *
 */

#ifndef INC_PARAMS_H_
#define INC_PARAMS_H_

#include "lookup.h"

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

// General
#define OFFSET_AVERAGE_SAMPLES  50     //!< Number of samples to average to determine i_pack offset
#define EEPROM_WRITE_PERIOD     60000   //!< Period to write stats to eeprom [msec]
#define IDLE_TIME_LIM           240     //!< Time limit to wait before considering car "idle" [sec]
#define BMS_UPDATE_IDLE_STATE_PERIOD    100 //!< Period to update idle status
#define SOC_FILTER_CONSTANT     0.9     //!< Alpha constant for SOC low pass filter when in current failsafe

#define INVERTER_SDC_COMMAND    0x10    //!< Command for the inverter to open the SDC
#define I_SENSE_GAIN            4.00   //!< Current Sensor Sensitivity [mV/Amp]

#define TEMPERATURE_MEASURE_PERIOD  1000    //!< Time period to read temperatures [ms]

//-----------------------------------------------------------------------------
// Charger
// The charge voltage and current limits must be sent in pairs to the 0.2 V and
// 0.05 amp that work with the charger. Not all pairs work.
//-----------------------------------------------------------------------------
#define CHARGER_FAIL_TIMEOUT    5000    //!< Time limit to wait for charger comm on bootup [ms]
#define CHARGER_VOLTAGE_LIMIT   1248    //!< [0.2 V] 1248 = 249.6 V
#define CHARGER_CURRENT_LIMIT   320     //!< [0.05 Amps] 320 = 16 amps

//-----------------------------------------------------------------------------
// Message timeouts
//-----------------------------------------------------------------------------
#define CHARGER_MSG_TIMEOUT     5000    //!< Time limit to wait between charger messages [ms]
#define ECU_MSG_TIMEOUT         5000    //!< Time limit to wait between ecu messages [ms]
#define AUX_FLAGS_MSG_TIMEOUT   5000    //!< Time limit [ms]

// Cell offsets
#define C_I_0_VOLTAGE_OFFSET 1420  //!< [100 uV]
#define C_I_10_VOLTAGE_OFFSET 1260  //!< [100 uV]
#define C_I_11_VOLTAGE_OFFSET 1260 //!< [100 uV]

// ----------------------------------------------------------------------------
// Current Limit Parameters
// ----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Cell Temperature Limits
//-----------------------------------------------------------------------------
#define CELL_OVERTEMPERATURE_LIMIT 60  //!<  [deg C]
#define CELL_UNDERTEMPERATURE_LIMIT -20  //!<  [deg C]
#define CELL_CRITICAL_TEMPERATURE_LIMIT 75  //!<  [deg C]
#define CELL_TEMPERATURE_OORH_LIMIT 24400  //!<  [100 uV]
#define CELL_TEMPERATURE_OORL_LIMIT 13000  //!<  [100 uV]
#define CELL_TEMPERATURE_INVALID_SENSORS_LIMIT 10  //!< [num sensors]


//-----------------------------------------------------------------------------
// Cell Closed Voltage Limits
//-----------------------------------------------------------------------------
#define CELL_VOLTAGE_OORL_LIMIT 5000  //!< Fault limit for OORL CCV warning [100 uV]
#define CELL_VOLTAGE_OORH_LIMIT 60000  //!< Fault limit for OORH CCV warning [100 uV]
#define CELL_VOLTAGE_DEVIATION_LIMIT 1000  //!< Fault limit for deviations between CCV warning [100 uV]
#define CELL_VOLTAGE_MIN_LIMIT 20000  //!< Fault limit for absolute minimum CCV warning [100 uV]
#define CELL_VOLTAGE_MIN_FAILSAFE_LIMIT 25000  //!< Fault limit for absolute minimum CCV warning (Failsafe mode) [100 uV]
#define CELL_VOLTAGE_MAX_LIMIT 42500  //!< Fault limit for absolute maximum CCV warning [100 uV]
#define CELL_VOLTAGE_MAX_FAILSAFE_LIMIT 41500  //!< Fault limit for absolute maximum CCV warning (Failsafe mode) [100 uV]


//-----------------------------------------------------------------------------
// Current Sensor Limits
//-----------------------------------------------------------------------------
#define I_SENSE_OORH_VOLTAGE 4.9f  //!< The max in-range value of the ADC [Volts]
#define I_SENSE_OORL_VOLTAGE 0.1f  //!< The min in-range value of the ADC [Volts]


//-----------------------------------------------------------------------------
// SOC Limits
//-----------------------------------------------------------------------------
#define SOC_100_OCV             42000   //!< Open circuit voltage for 100% SOC [0.1mV]
#define SOC_000_OCV             25000   //!< Open circuit voltage for 0% SOC [0.1mV]

//-----------------------------------------------------------------------------
// Overcurrent checks
//-----------------------------------------------------------------------------
#define CL_CHECK_PERIOD         5       //!< How often to integrate [ms]
#define CCL_MAX_OVER            100     //!< [A * ms]
#define DCL_MAX_OVER            100     //!< [A * ms]
#define CCL_LEAK_RATE           1       //!< [A * ms / ms]
#define DCL_LEAK_RATE           1       //!< [A * ms / ms]
#define I_MAX_ERROR             15       //!< The allowed error when current should be 0
#define I_OFF_DELAY             2000    //!< ms after opening AIRs before current should be 0.

//-----------------------------------------------------------------------------
// Cell Balacing
//-----------------------------------------------------------------------------
#define BALANCING_VLTG_LIMIT    39000   //!< Minimum voltage to discharge a cell [0.1mV]
#define BALANCING_VLTG_DIFF       100   //!< Minimum difference between max and average voltage to enable discharge [0.1mV]

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------
extern const lut_2d_t CR_CH_LUT;
extern const lut_2d_t CR_DCH_LUT;
extern const lut_1d_t CT_LUT;
extern const lut_1d_t SOC_LUT;
extern const lut_1d_t OCV_LUT;

#endif /* INC_PARAMS_H_ */
