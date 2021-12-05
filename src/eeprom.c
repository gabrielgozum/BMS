///**
// * @brief EEPROM Interface
// *
// * This file includes functions to initialize the EEPROM peripheral,
// * store, and retrieve values.
// *
// */
//
//#include "stdbool.h"
//#include "stdint.h"
//#include "driverlib/eeprom.h"
//#include "driverlib/sysctl.h"
//#include "inc/hw_memmap.h"
//#include "bms.h"
//#include "eeprom.h"
//
//
///**
// * @brief Initilizes the EEPROM module.
// */
//void eeprom_init(void)
//{
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
//    EEPROMInit();
//}
//
///**
// * @brief Clears the EEPROM struct and memory.
// *
// * Only call to reset all persistent data.
// */
//void eeprom_clear(eeprom_stats *stats)
//{
//        stats->bms_runtime = 0;
//        stats->bms_powerups = 0;
//        stats->pack_cycles = 0;
//        stats->ct_avg_gt_45_time = 0;
//        stats->ct_avg_gt_60_time = 0;
//        stats->ct_avg_lt_neg_20_time = 0;
//        stats->ccv_avg_over_time = 0;
//        stats->ccv_avg_under_time = 0;
//        stats->dcl_violations = 0;
//        stats->ccl_violations = 0;
//        stats->pack_runtime = 0;
//        stats->reserved = 0;
//
//        // write zero data to EEPROM
//        eeprom_stats_write(stats);
//}
//
///**
// * @brief Clears pack capacity and memory.
// *
// * Only call to reset persistent data.
// */
//void eeprom_soc_clear(void)
//{
//    bms.soc = 0.0;
//    eeprom_write_array((uint32_t*)&(bms.soc), EEPROM_PACK_SOC_ADDR, EEPROM_PACK_SOC_LEN);
//}
//
//void eeprom_capacity_clear(void)
//{
//    bms.pack_capacity = 0.0;
//    eeprom_write_array((uint32_t*)&(bms.soc), EEPROM_PACK_CAPACITY_ADDR, EEPROM_PACK_CAPACITY_LEN);
//}
//
///**
// * @brief Loads stats from EEPROM.
// *
// * Stores in given struct.
// */
//void eeprom_stats_load(eeprom_stats *stats)
//{
//    // Round down, parts of unused reserved field will be cut off.
//    uint16_t length = (sizeof(eeprom_stats) / 4) * 4;
//    eeprom_read_array((uint32_t *)stats, EEPROM_STATS_ADDR, length);
//}
//
///**
// * @brief Stores stats struct in EEPROM.
// */
//void eeprom_stats_write(eeprom_stats *stats)
//{
//    // Round down, parts of unused reserved field will be cut off.
//    uint16_t length = (sizeof(eeprom_stats) / 4) * 4;
//    eeprom_write_array((uint32_t *)stats, EEPROM_STATS_ADDR, length);
//}
//
///**
// * @brief Writes data to EEPROM.
// *
// * EEPROM is programmed in 32 bit chunks.
// *
// * @param data_in Array of data to write to EEPROM.
// * @param address The base address to start writing at. Must be a multiple of 4.
// * @param length The number of elements in the array.
// */
//void eeprom_write_array(uint32_t *data_in, uint32_t address, uint32_t length)
//{
//    if (address % 4 == 0)
//    {
//        // takes length in bytes, so multiply by 4 for uint32
//        EEPROMProgram(data_in, address, length * 4);
//    }
//}
//
///**
// * @brief reads memory from EEPROM.
// *
// * EEPROM is read in 32 bit chunks.
// *
// * @param[out] data_out Array to store retrieved data in.
// * @param address The base address to start at. Must be a multiple of 4.
// * @param length The number of elements in the array.
// */
//void eeprom_read_array(uint32_t *data_out, uint32_t address, uint32_t length)
//{
//    if (address % 4 == 0)
//    {
//        // takes length in bytes, so multiply by 4 for uint32
//        EEPROMRead(data_out, address, length * 4);
//    }
//}
//
///**
// * @brief Reads one 32 bit block from EEPROM.
// *
// * @param address The base address to start reading at. Must be a multiple of 4.
// * @return the data at the given address.
// */
//uint32_t eeprom_read(uint32_t address)
//{
//    uint32_t data[1];
//    eeprom_read_array(data, address, 1);
//    return data[0];
//}
//
///**
// * @brief Writes one 32 bit block to EEPROM.
// *
// * @param address The base address to start reading at. Must be a multiple of 4.
// * @param data The data to write.
// */
//void eeprom_write(uint32_t address, uint32_t data)
//{
//    uint32_t data_array[1] = {data};
//    eeprom_write_array(data_array, address, 1);
//}
