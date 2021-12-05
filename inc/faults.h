/*
 * BMS faults interface to cell level faults.
 */

#ifndef INC_FAULTS_H_
#define INC_FAULTS_H_

#include <stdbool.h>


// ----------------------------------------------------------------------------
// Fault Interface
// ----------------------------------------------------------------------------
bool is_warning_active(void);
bool is_cell_temperature_valid(uint8_t cell_id);
bool is_cell_voltage_valid(uint8_t cell_id);

void set_cell_overtemperature_fault(uint8_t cell_id, int8_t temperature);
void clear_cell_overtemperature_fault(uint8_t cell_id);
bool is_cell_overtemperature_fault_active(uint8_t cell_id);

void set_cell_undertemperature_fault(uint8_t cell_id, int8_t temp);
void clear_cell_undertemperature_fault(uint8_t cell_id);
bool is_cell_undertemperature_fault_active(uint8_t cell_id);

void set_cell_temperature_above_60_fault(uint8_t cell_id, int8_t temperature);
void clear_cell_temperature_above_60_fault(uint8_t cell_id);
bool is_cell_temperature_above_60_fault_active(uint8_t cell_id);

void set_cell_temperature_oorl_fault(uint8_t cell_id, uint16_t voltage);
void clear_cell_temperature_oorl_fault(uint8_t cell_id);
bool is_cell_temperature_oorl_fault_active(uint8_t cell_id);

void set_cell_temperature_oorh_fault(uint8_t cell_id, uint16_t voltage);
void clear_cell_temperature_oorh_fault(uint8_t cell_id);
bool is_cell_temperature_oorh_fault_active(uint8_t cell_id);

void set_cell_overvoltage_fault(uint8_t cell_id, uint16_t voltage);
void clear_cell_overvoltage_fault(uint8_t cell_id);
bool is_cell_overvoltage_fault_active(uint8_t cell_id);

void set_cell_undervoltage_fault(uint8_t cell_id, uint16_t voltage);
void clear_cell_undervoltage_fault(uint8_t cell_id);
bool is_cell_undervoltage_fault_active(uint8_t cell_id);

void set_cell_highimpedence(uint8_t cell_id, uint16_t resistance);
void clear_cell_highimpedence_fault(uint8_t cell_id);
bool is_cell_highimpedence_active_fault(uint8_t cell_id);

void set_cell_voltage_oorl_fault(uint8_t cell_id, uint16_t voltage);
void clear_cell_voltage_oorl_fault(uint8_t cell_id);
bool is_cell_voltage_oorl_fault_active(uint8_t cell_id);

void set_cell_voltage_oorh_fault(uint8_t cell_id, uint16_t voltage);
void clear_cell_voltage_oorh_fault(uint8_t cell_id);
bool is_cell_voltage_oorh_fault_active(uint8_t cell_id);

#endif /* INC_FAULTS_H_ */
