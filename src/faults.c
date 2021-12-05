
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "fault_manager.h"
#include "params.h"

// bit flags for each fault for each cell
static uint8_t cell_overtemperature_flags[8];
static uint8_t cell_undertemperature_flags[8];
static uint8_t cell_critical_temperature_flags[8];
static uint8_t cell_temperature_oorl_flags[8];
static uint8_t cell_temperature_oorh_flags[8];
static uint8_t cell_overvoltage_flags[8];
static uint8_t cell_undervoltage_flags[8];
static uint8_t cell_voltage_oorl_flags[8];
static uint8_t cell_voltage_oorh_flags[8];
static uint8_t cell_highimpedence_flags[8];

static uint8_t num_cell_overtemperature = 0;
static uint8_t num_cell_undertemperature = 0;
static uint8_t num_cell_critical_temperature = 0;
static uint8_t num_cell_temperature_oorl = 0;
static uint8_t num_cell_temperature_oorh = 0;
static uint8_t num_cell_overvoltage = 0;
static uint8_t num_cell_undervoltage = 0;
static uint8_t num_cell_voltage_oorl = 0;
static uint8_t num_cell_voltage_oorh = 0;
static uint8_t num_cell_highimpedance = 0;


bool is_warning_active(void)
{
    return (fault_is_wChargerMIA()
            || fault_is_wChargerFault()
            || fault_is_wisoSPIPECError()
            || fault_is_wSelfTestError()
            || fault_is_wCellUndervoltage()
            || fault_is_wCellOvervoltage()
            || fault_is_wCellOvertemperature()
            || fault_is_wCellUndertemperature()
            || fault_is_wInverterSDCOpenRequest()
            || fault_is_wCellTemperatureLostSensors()
            || fault_is_wChargeCurrentLimitOver()
            || fault_is_wDischargeCurrentLimitOver()
            || fault_is_wChargeCompleted()
            || fault_is_wDischargeCompleted()
    );
}


static bool set_fault_active(uint8_t cell_id, uint8_t a[], uint8_t *num)
{
    uint8_t byte_i = cell_id / 8;
    uint8_t bit_i = cell_id % 8;

    bool already_active = a[byte_i] & (1 << bit_i);

    a[byte_i] |= 1 << bit_i;

    if (*num < UCHAR_MAX && !already_active)
    {
        (*num)++;
    }

    return already_active;
}


static bool clear_fault(uint8_t cell_id, uint8_t a[], uint8_t *num)
{
    uint8_t byte_i = cell_id / 8;
    uint8_t bit_i = cell_id % 8;

    bool already_active = a[byte_i] & (1 << bit_i);

    a[byte_i] &= ~(1 << bit_i);

    if (*num > 0 && already_active)
    {
        (*num)--;
    }

    return (*num == 0) && already_active;
}


static bool is_active_fault(uint8_t cell_id, uint8_t a[])
{
    uint8_t byte_i = cell_id / 8;
    uint8_t bit_i = cell_id % 8;
    return (a[byte_i] & (1 << bit_i)) != 0;
}


static void check_for_lost_temp_sensors(void)
{
    uint8_t lost_sensors = num_cell_temperature_oorl + num_cell_temperature_oorh;

    if (lost_sensors > CELL_TEMPERATURE_INVALID_SENSORS_LIMIT)
    {
        fault_set_wCellTemperatureLostSensors(lost_sensors);
    }
    else
    {
        fault_clear_wCellTemperatureLostSensors();
    }
}

void set_cell_overtemperature_fault(uint8_t cell_id, int8_t temp)
{
    if (!set_fault_active(cell_id, cell_overtemperature_flags, &num_cell_overtemperature))
    {
        fault_set_wCellOvertemperature(cell_id, temp);
    }
}


void clear_cell_overtemperature_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_overtemperature_flags, &num_cell_overtemperature))
    {
        fault_clear_wCellOvertemperature();
    }
}


bool is_cell_overtemperature_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_overtemperature_flags);
}


void set_cell_undertemperature_fault(uint8_t cell_id, int8_t temperature)
{
    if (!set_fault_active(cell_id, cell_undertemperature_flags, &num_cell_undertemperature))
    {
        fault_set_wCellUndertemperature(cell_id, temperature);
    }
}


void clear_cell_undertemperature_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_undertemperature_flags, &num_cell_undertemperature))
    {
        fault_clear_wCellUndertemperature();
    }
}


bool is_cell_undertemperature_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_undertemperature_flags);
}


void set_cell_temperature_above_60_fault(uint8_t cell_id, int8_t temperature)
{
    if (!set_fault_active(cell_id, cell_critical_temperature_flags, &num_cell_critical_temperature))
    {
        fault_set_wCellTemperatureAbove60degC(cell_id, temperature);
    }

    check_for_lost_temp_sensors();
}


void clear_cell_temperature_above_60_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_critical_temperature_flags, &num_cell_critical_temperature))
    {
        fault_clear_wCellTemperatureAbove60degC();
    }
}


bool is_cell_temperature_above_60_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_critical_temperature_flags);
}


void set_cell_overvoltage_fault(uint8_t cell_id, uint16_t voltage)
{
    if (!set_fault_active(cell_id, cell_overvoltage_flags, &num_cell_overvoltage))
    {
        fault_set_wCellOvervoltage(cell_id, voltage);
    }
}


void clear_cell_overvoltage_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_overvoltage_flags, &num_cell_overvoltage))
    {
        fault_clear_wCellOvervoltage();
    }
}


bool is_cell_overvoltage_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_overvoltage_flags);
}


void set_cell_undervoltage_fault(uint8_t cell_id, uint16_t voltage)
{
    if (!set_fault_active(cell_id, cell_undervoltage_flags, &num_cell_undervoltage))
    {
        fault_set_wCellUndervoltage(cell_id, voltage);
    }
}


void clear_cell_undervoltage_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_undervoltage_flags, &num_cell_undervoltage))
    {
        fault_clear_wCellUndervoltage();
    }
}


bool is_cell_undervoltage_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_undervoltage_flags);
}


void set_cell_highimpedence_active(uint8_t cell_id, uint8_t resistance)
{
    if (!set_fault_active(cell_id, cell_highimpedence_flags, &num_cell_highimpedance))
    {
        fault_set_aCellHighImpedance(cell_id, resistance);
    }
}


void clear_cell_highimpedence_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_highimpedence_flags, &num_cell_highimpedance))
    {
        fault_clear_aCellHighImpedance();
    }
}


bool is_cell_highimpedence_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_highimpedence_flags);
}


void set_cell_temperature_oorl_fault(uint8_t cell_id, uint16_t voltage)
{
    if (!set_fault_active(cell_id, cell_temperature_oorl_flags, &num_cell_temperature_oorl))
    {
        fault_set_aCellTemperatureOORL(cell_id, voltage);
    }

    check_for_lost_temp_sensors();
}


void clear_cell_temperature_oorl_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_temperature_oorl_flags, &num_cell_temperature_oorl))
    {
        fault_clear_aCellTemperatureOORL();
    }
}


bool is_cell_temperature_oorl_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_temperature_oorl_flags);
}


void set_cell_temperature_oorh_fault(uint8_t cell_id, uint16_t voltage)
{
    if (!set_fault_active(cell_id, cell_temperature_oorh_flags, &num_cell_temperature_oorh))
    {
        fault_set_aCellTemperatureOORH(cell_id, voltage);
    }

    check_for_lost_temp_sensors();
}


void clear_cell_temperature_oorh_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_temperature_oorh_flags, &num_cell_temperature_oorh))
    {
        fault_clear_aCellTemperatureOORH();
    }
}


bool is_cell_temperature_oorh_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_temperature_oorh_flags);
}


void set_cell_voltage_oorl_fault(uint8_t cell_id, uint16_t voltage)
{
    if (!set_fault_active(cell_id, cell_voltage_oorl_flags, &num_cell_voltage_oorl))
    {
        fault_set_cCellVoltageOORL(cell_id, voltage);
    }
}


void clear_cell_voltage_oorl_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_voltage_oorl_flags, &num_cell_voltage_oorl))
    {
        fault_clear_cCellVoltageOORL();
    }
}


bool is_cell_voltage_oorl_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_voltage_oorl_flags);
}


void set_cell_voltage_oorh_fault(uint8_t cell_id, uint16_t voltage)
{
    if (!set_fault_active(cell_id, cell_voltage_oorh_flags, &num_cell_voltage_oorh))
    {
        fault_set_cCellVoltageOORH(cell_id, voltage);
    }
}


void clear_cell_voltage_oorh_fault(uint8_t cell_id)
{
    if (clear_fault(cell_id, cell_voltage_oorh_flags, &num_cell_voltage_oorh))
    {
    fault_clear_cCellVoltageOORH();
    }
}


bool is_cell_voltage_oorh_fault_active(uint8_t cell_id)
{
    return is_active_fault(cell_id, cell_voltage_oorh_flags);
}


bool is_cell_voltage_valid(uint8_t cell_id)
{
    return (!is_cell_voltage_oorh_fault_active(cell_id)
            && !is_cell_voltage_oorl_fault_active(cell_id));
}


bool is_cell_temperature_valid(uint8_t cell_id)
{
    return (!is_cell_temperature_oorh_fault_active(cell_id)
            && !is_cell_temperature_oorl_fault_active(cell_id));
}

