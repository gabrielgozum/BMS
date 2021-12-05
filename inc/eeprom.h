/**
 * @brief EEPROM Interface
 *
 * This file includes functions to initialize the EEPROM peripheral,
 * store, and retrieve values.
 *
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------

// Address locations for storing parameters
#define EEPROM_PACK_SOC_ADDR        (0x0)
#define EEPROM_PACK_SOC_LEN         4
#define EEPROM_PACK_CAPACITY_ADDR   (EEPROM_PACK_SOC_ADDR + EEPROM_PACK_SOC_LEN*4)
#define EEPROM_PACK_CAPACITY_LEN    4
#define EEPROM_STATS_ADDR           (EEPROM_PACK_CAPACITY_ADDR + EEPROM_PACK_CAPACITY_LEN*4)

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------

typedef struct
{
    uint32_t bms_runtime;               //!< Runtime of BMS [minutes]
    uint16_t bms_powerups;              //!< Number of BMS Powerups
    uint16_t pack_cycles;               //!< Capacity Cycles of Pack
    uint16_t ct_avg_gt_45_time;         //!< Time Average Cell Temperature has been above 45 deg C [minutes]
    uint16_t ct_avg_gt_60_time;         //!< Time Average Cell Temperature has been above 60 deg C [minutes]
    uint16_t ct_avg_lt_neg_20_time;     //!< Time Average Cell Temperature has been below -20 deg C [minutes]
    uint16_t ccv_avg_over_time;         //!< Time Average Closed Cell Voltage has been above limit [minutes]
    uint16_t ccv_avg_under_time;        //!< Time Average Closed Cell Voltage has been below limit [minutes]
    uint16_t dcl_violations;            //!< Number of Discharge Current Limit Violations
    uint16_t ccl_violations;            //!< Number of Charge Current Limit Violations
    uint32_t pack_runtime;              //!< Runtime of Pack (Time AIRs have been closed) [minutes]

    uint32_t reserved;    //!< Used to make stats a clean multiple of 32
} eeprom_stats;

void eeprom_init(void);
void eeprom_clear(eeprom_stats *stats);
void eeprom_soc_clear(void);
void eeprom_capacity_clear(void);
void eeprom_stats_load(eeprom_stats *stats);
void eeprom_stats_write(eeprom_stats *stats);
void eeprom_write_array(uint32_t *data_in, uint32_t address, uint32_t length);
void eeprom_read_array(uint32_t *data_out, uint32_t address, uint32_t length);
uint32_t eeprom_read(uint32_t address);
void eeprom_write(uint32_t address, uint32_t data);

#endif /* INC_EEPROM_H_ */
