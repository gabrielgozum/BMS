/**
 * @brief CAN Interface
 *
 * This file includes functions to initialize the CAN peripheral, send and
 * receive CAN messages. Receiving functions will parse the CAN data and place
 * into the appropriate place.
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <stdbool.h>

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

#define CAN_TIMEOUT_MAX 2000 //!< Time to wait for message from car before switching to charger CAN
#define BOOTLOADER_DEVICE_ID 0x20000 //!< Base id for bootloader messages

//-----------------------------------------------------------------------------
// CAN IDs, MSG OBJ IDs, Lengths - MSG OBJ IDs are 1 - 32
//-----------------------------------------------------------------------------

// BMS Vitals - Tx
#define CAN_VITALS_OBJ_ID 1
#define CAN_VITALS_OBJ_ID_M (1 << (CAN_VITALS_OBJ_ID - 1))
#define CAN_VITALS_MSG_ID 0x040
#define CAN_VITALS_LEN 8

// BMS Pack Stats - Tx
#define CAN_PACK_STATS_OBJ_ID (CAN_VITALS_OBJ_ID + 1)
#define CAN_PACK_STATS_OBJ_ID_M (1 << (CAN_PACK_STATS_OBJ_ID - 1))
#define CAN_PACK_STATS_MSG_ID 0x041
#define CAN_PACK_STATS_LEN 8

// Charger Command - Tx
#define CAN_CHARGER_CMD_OBJ_ID (CAN_PACK_STATS_OBJ_ID + 1)
#define CAN_CHARGER_CMD_OBJ_ID_M (1 << (CAN_CHARGER_CMD_OBJ_ID - 1))
#define CAN_CHARGER_CMD_MSG_ID 0x18FF4AEF
#define CAN_CHARGER_CMD_LEN 8

#define CAN_CELL_BROADCAST_1_OBJ_ID (CAN_CHARGER_CMD_OBJ_ID + 1)
#define CAN_CELL_BROADCAST_1_OBJ_ID_M (1 << (CAN_CELL_BROADCAST_1_OBJ_ID - 1))
#define CAN_CELL_BROADCAST_1_MSG_ID 0x341
#define CAN_CELL_BROADCAST_1_LEN 8

#define CAN_CELL_BROADCAST_2_OBJ_ID (CAN_CELL_BROADCAST_1_OBJ_ID + 1)
#define CAN_CELL_BROADCAST_2_OBJ_ID_M (1 << (CAN_CELL_BROADCAST_2_OBJ_ID - 1))
#define CAN_CELL_BROADCAST_2_MSG_ID 0x342
#define CAN_CELL_BROADCAST_2_LEN 6

// BMS Config - Rx
#define CAN_CONFIG_OBJ_ID (CAN_CELL_BROADCAST_2_OBJ_ID + 1)
#define CAN_CONFIG_OBJ_ID_M (1 << (CAN_CONFIG_OBJ_ID - 1))
#define CAN_CONFIG_MSG_ID 0x241
#define CAN_CONFIG_LEN 8

// Charger Status - Rx
#define CAN_CHARGER_STATUS_OBJ_ID (CAN_CONFIG_OBJ_ID + 1)
#define CAN_CHARGER_STATUS_OBJ_ID_M (1 << (CAN_CHARGER_STATUS_OBJ_ID - 1))
#define CAN_CHARGER_STATUS_MSG_ID 0x18FF4049
#define CAN_CHARGER_STATUS_LEN 8

// SOC/SOE
#define CAN_SOC_OBJ_ID (CAN_CHARGER_STATUS_OBJ_ID + 1)
#define CAN_SOC_OBJ_ID_M (1 << (CAN_SOC_OBJ_ID - 1))
#define CAN_SOC_MSG_ID 0x043
#define CAN_SOC_LEN 8

#define CAN_CELL_BROADCAST_3_OBJ_ID (CAN_SOC_OBJ_ID + 1)
#define CAN_CELL_BROADCAST_3_OBJ_ID_M (1 << (CAN_CELL_BROADCAST_3_OBJ_ID - 1))
#define CAN_CELL_BROADCAST_3_MSG_ID 0x343
#define CAN_CELL_BROADCAST_3_LEN 8

// Cell Voltage Stats
#define CAN_STATS_CELL_VOLTAGE_OBJ_ID (CAN_CELL_BROADCAST_3_OBJ_ID + 1)
#define CAN_STATS_CELL_VOLTAGE_OBJ_ID_M (1 << (CAN_STATS_ELL_VOLTAGE_OBJ_ID - 1))
#define CAN_STATS_CELL_VOLTAGE_MSG_ID 0x140
#define CAN_STATS_CELL_VOLTAGE_LEN 8

// Cell Temperature Stats
#define CAN_STATS_CELL_TEMPERATURE_OBJ_ID (CAN_STATS_CELL_VOLTAGE_OBJ_ID + 1)
#define CAN_STATS_CELL_TEMPERATURE_OBJ_ID_M (1 << (CAN_STATS_CELL_TEMPERATURE_OBJ_ID - 1))
#define CAN_STATS_CELL_TEMPERATURE_MSG_ID 0x141
#define CAN_STATS_CELL_TEMPERATURE_LEN 5

// Open Cell Voltage Stats
#define CAN_STATS_OCV_OBJ_ID (CAN_STATS_CELL_TEMPERATURE_OBJ_ID + 1)
#define CAN_STATS_OCV_OBJ_ID_M (1 << (CAN_STATS_OCV_OBJ_ID - 1))
#define CAN_STATS_OCV_MSG_ID 0x142
#define CAN_STATS_OCV_LEN 8

// Cell Discharge Resistance Stats
#define CAN_STATS_CELL_DCH_RES_OBJ_ID (CAN_STATS_OCV_OBJ_ID + 1)
#define CAN_STATS_CELL_DCH_RES_OBJ_ID_M (1 << (CAN_STATS_CELL_DCH_RES_OBJ_ID - 1))
#define CAN_STATS_CELL_DCH_RES_MSG_ID 0x144
#define CAN_STATS_CELL_DCH_RES_LEN 8

// Inverter Faults
#define CAN_IFR_FAULT_OBJ_ID (CAN_STATS_CELL_DCH_RES_OBJ_ID + 1)
#define CAN_IFR_FAULT_OBJ_ID_M (1 << (CAN_IFR_FAULT_OBJ_ID - 1))
#define CAN_IFR_FAULT_MSG_ID 0xD0
#define CAN_IFR_FAULT_LEN 5

// FL Inverter Fault
#define CAN_IFL_FAULT_OBJ_ID (CAN_IFR_FAULT_OBJ_ID + 1)
#define CAN_IFL_FAULT_OBJ_ID_M (1 << (CAN_IFL_FAULT_OBJ_ID - 1))
#define CAN_IFL_FAULT_MSG_ID 0xD1
#define CAN_IFL_FAULT_LEN 5

// RL Inverter Fault
#define CAN_IRL_FAULT_OBJ_ID (CAN_IFL_FAULT_OBJ_ID + 1)
#define CAN_IRL_FAULT_OBJ_ID_M (1 << (CAN_IRL_FAULT_OBJ_ID - 1))
#define CAN_IRL_FAULT_MSG_ID 0xD2
#define CAN_IRL_FAULT_LEN 5

// RR Inverter Fault
#define CAN_IRR_FAULT_OBJ_ID (CAN_IRL_FAULT_OBJ_ID + 1)
#define CAN_IRR_FAULT_OBJ_ID_M (1 << (CAN_IRR_FAULT_OBJ_ID - 1))
#define CAN_IRR_FAULT_MSG_ID 0xD3
#define CAN_IRR_FAULT_LEN 5

// Bootloader ping - Rx
#define CAN_BOOTLOADER_PING_OBJ_ID (CAN_IRR_FAULT_OBJ_ID + 1)
#define CAN_BOOTLOADER_PING_OBJ_ID_M (1 << (CAN_BOOTLOADER_PING_OBJ_ID - 1))
#define CAN_BOOTLOADER_PING_MSG_ID 0x1F000000 | BOOTLOADER_DEVICE_ID
#define CAN_BOOTLOADER_PING_LEN 0

// Cell Charge Resistance Stats
#define CAN_STATS_CELL_CH_RES_OBJ_ID (CAN_BOOTLOADER_PING_OBJ_ID + 1)
#define CAN_STATS_CELL_CH_RES_OBJ_ID_M (1 << (CAN_STATS_CELL_CH_RES_OBJ_ID - 1))
#define CAN_STATS_CELL_CH_RES_MSG_ID 0x143
#define CAN_STATS_CELL_CH_RES_LEN 8

// AuxMCU Flags
#define CAN_AUX_FLAGS_OBJ_ID (CAN_STATS_CELL_CH_RES_OBJ_ID + 1)
#define CAN_AUX_FLAGS_OBJ_ID_M (1 << (CAN_AUX_FLAGS_OBJ_ID - 1))
#define CAN_AUX_FLAGS_MSG_ID 0x030
#define CAN_AUX_FLAGS_LEN 2

// BMS Version
#define CAN_BMS_VERSION_OBJ_ID (CAN_AUX_FLAGS_OBJ_ID + 1)
#define CAN_BMS_VERSION_OBJ_ID_M (1 << (CAN_BMS_VERSION_OBJ_ID - 1))
#define CAN_BMS_VERSION_MSG_ID 0x344
#define CAN_BMS_VERSION_LEN 8

// BMS Cell Balancing
#define CAN_CELL_BALANCING_OBJ_ID (CAN_BMS_VERSION_OBJ_ID + 1)
#define CAN_CELL_BALANCING_OBJ_ID_M (1 << (CAN_CELL_BALANCING_OBJ_ID - 1))
#define CAN_CELL_BALANCING_MSG_ID 0x345
#define CAN_CELL_BALANCING_LEN 8

// BMS Debug
#define CAN_DEBUG_OBJ_ID (CAN_CELL_BALANCING_OBJ_ID + 1)
#define CAN_DEBUG_OBJ_ID_M (1 << (CAN_DEBUG_OBJ_ID - 1))
#define CAN_DEBUG_MSG_ID 0x346
#define CAN_DEBUG_LEN 8

// BMS SOC Stats
#define CAN_STATS_SOC_OBJ_ID (CAN_DEBUG_OBJ_ID + 1)
#define CAN_STATS_SOC_OBJ_ID_M (1 << (CAN_STATS_SOC_OBJ_ID - 1))
#define CAN_STATS_SOC_MSG_ID 0x147
#define CAN_STATS_SOC_LEN 8

// BMS Capacity Stats
#define CAN_STATS_CAPACITY_OBJ_ID (CAN_STATS_SOC_OBJ_ID + 1)
#define CAN_STATS_CAPACITY_OBJ_ID_M (1 << CAN_STATS_CAPACITY_OBJ_ID - 1))
#define CAN_STATS_CAPACITY_MSG_ID 0x146
#define CAN_STATS_CAPACITY_LEN 8

// BMS Faults
#define CAN_FAULTS_OBJ_ID (CAN_STATS_CAPACITY_OBJ_ID + 1)
#define CAN_FAULTS_OBJ_ID_M (1 << CAN_FAULTS_OBJ_ID - 1)
#define CAN_FAULTS_MSG_ID 0x145
#define CAN_FAULTS_LEN 8

// BMS Faults Matrix
#define CAN_FAULTS_MATRIX_OBJ_ID (CAN_FAULTS_OBJ_ID + 1)
#define CAN_FAULTS_MATRIX_OBJ_ID_M (1 << CAN_FAULTS_MATRIX_ID - 1)
#define CAN_FAULTS_MATRIX_MSG_ID 0x246
#define CAN_FAULTS_MATRIX_LEN 8

//-----------------------------------------------------------------------------
// Gains and Offsets
//-----------------------------------------------------------------------------

// BMS Vitals
#define CAN_VITALS_CELL_V_GAIN      10

// Pack Stats
#define CAN_PACK_I_GAIN             10
#define CAN_PACK_R_GAIN             1000

// Charger Command
#define CAN_CHARGER_I_GAIN          20
#define CAN_CHARGER_V_GAIN          5

// Broadcasts
#define CAPACITY_GAIN_DIV           360000
#define ENERGY_GAIN_DIV             360
#define SOC_GAIN                    200
#define SOE_GAIN                    50

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------
void can_init(void);
void can_discharge_configure(void);
uint8_t can_calc_chrgr_checksum(uint8_t data[], uint32_t message_id);
bool can_is_rx_chrgr_checksum_valid(uint8_t data[]);
void can_check_timeouts(void);
void can_data_handler(void);
void can_decrement_count(void);
void set_bms_cell_fault_multiplexer(uint8_t* a, uint8_t mux_value);

// sending messages
void can_send_fault_matrix(void);
void can_send_cell_broadcast_1(void);
void can_send_cell_broadcast_2(void);
void can_send_cell_broadcast_3(void);
void can_send_debug(void);
void can_send_stats_soc(void);
void can_send_stats_capacity(void);
void can_send_stats_cell_temperature(void);
void can_send_stats_cell_voltage(void);
void can_send_stats_ocv(void);
void can_send_stats_cell_dch_res(void);
void can_send_stats_cell_ch_res(void);
void can_send_bms_version(void);
void can_send_cell_balancing(void);
void can_send_vitals(void);
void can_send_pack_stats(void);
void can_send_charger_command(void);
void can_send_fault(uint8_t *data);

#endif /* INC_CAN_H_ */
