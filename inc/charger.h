/**
 * @brief Charger Interface
 *
 * This file includes functions to interface with the charger
 *
 */

#ifndef INC_CHARGER_H_
#define INC_CHARGER_H_

#include <stdbool.h>
#include <stdint.h>
#include "scheduler.h"

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

#define CHARGER_TIME_LIMIT                  (120 * 60 * TICKS_PER_SECOND)
#define CHARGER_VOLTAGE_LIMIT1 1632 // 1248 // Factor of 0.2V/div
#define CHARGER_CURRENT_LIMIT1 320  // Factor of .05A/div
#define CHARGER_VOLTAGE_LIMIT2 1636 // 1254
#define CHARGER_CURRENT_LIMIT2 100

// values for charge_enable
#define CHARGER_DISABLED                    0
#define CHARGER_ENABLED                     1
#define CHARGER_ENABLED_ERROR               2
#define CHARGER_ENABLED_NA                  3

// values for charge_complete
#define CHARGER_NOT_COMPLETE                0
#define CHARGER_COMPLETE                    1
#define CHARGER_COMPLETE_ERROR              2
#define CHARGER_COMPLETE_NA                 3

// values for charge_system_fault
#define CHARGER_NO_FAULT                    0
#define CHARGER_FAULT_ACTIVE                1
#define CHARGER_FAULT_ERROR                 2
#define CHARGER_FAULT_NA                    3

// values for ignition_status
#define CHARGER_IGNITION_STATUS_OFF         0
#define CHARGER_IGNITION_STATUS_ON          1
#define CHARGER_IGNITION_STATUS_ERROR       2
#define CHARGER_IGNITION_STATUS_NA          3

// values for charger_state
#define CHARGER_STATE_NC                    0
#define CHARGER_STATE_STANDBY               1
#define CHARGER_STATE_CHARGING              2
#define CHARGER_STATE_NA                    7

// values for fault_severity
#define CHARGER_FAULT_SEVERITY_NONE         0
#define CHARGER_FAULT_SEVERITY_WARNING      1
#define CHARGER_FAULT_SEVERITY_SOFT_FAIL    5
#define CHARGER_FAULT_SEVERITY_HARD_FAIL    6
#define CHARGER_FAULT_SEVERITY_NA           7

#define CHARGER_MESSAGE_COUNTER_NA          0xf
#define CHARGER_MESSAGE_CHECKSUM_NA         0xf

// values for charging mode
#define CONSTANT_CURRENT                    0
#define CONSTANT_VOLTAGE                    1
#define BALANCE                             2
#define COMPLETED                           3

typedef struct
{
    // received CAN signals
    uint8_t input_voltage;              //!< [Vrms]
    uint8_t output_voltage;             //!< [VDC]
    uint8_t input_current_limit_max;    //!< [Arms]
    uint8_t input_current;              //!< [Arms]
    uint8_t output_current;             //!< [Amps]
    int8_t temperature;                 //!< [deg C]
    uint8_t ignition_status;            //!< see defines for values
    uint8_t charger_state;              //!< see defines for values
    uint8_t fault_severity;             //!< see defines for values

    bool rx_msg_checksum_faulted;
    bool rx_msg_counter_faulted;

    // sent CAN signals
    uint8_t charge_enable;              //!< Charger Enable (see defines for values)
    uint8_t charge_complete;            //!< Controls LED on Charger (see defines for values)
    uint8_t charge_system_fault;        //!< Controls LED on Charger (see defines for values)
    uint16_t voltage_limit;             //!< [0.2 V]
    uint16_t current_limit;             //!< [0.05 A]

    uint8_t charging_mode;              //!< CC mode, CV mode, Balancing mode, or Completed
    uint32_t time_limit;                //!< time limit for charging [ticks]

} charger_t;

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------

extern charger_t charger;

/**
 * Send command values based on which mode the charger is in. Best lithium ion
 * strategy is constant current until 4.1V, then switch to constant voltage
 * until charging current is little to none.
 */
void charger_set_command_values(void);

/**
 * Analyze the data received from the charger and compare to charging request.
 * This function does not process the Rx message counter or checksum.
 */
void charger_process_rx_data(void);

/**
 * Set charger_enable to CHARGER_ENABLED. This signal is sent to the charger
 * over CAN.
 */
void charger_enable(void);

/**
 * Set charger_enable to CHARGER_DISABLED. This signal is sent to the charger
 * over CAN.
 */
void charger_disable(void);

#endif /* INC_CHARGER_H_ */
