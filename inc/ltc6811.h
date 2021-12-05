/**
 * This file includes defines, declarations, and includes.
 *
 */

#ifndef CODE_LTC6811_BMS_H_
#define CODE_LTC6811_BMS_H_

#include <stdbool.h>
#include <stdint.h>
#include "bms.h"

// ----------------------------------------------------------------------------
//
// Defines
//
// ----------------------------------------------------------------------------
//#define TOTAL_IC 1 moved to bms.h
#define MD_422HZ_1KHZ 0
#define MD_27KHZ_14KHZ 1
#define MD_7KHZ_3KHZ 2
#define MD_26HZ_2KHZ 3

#define ADC_OPT_ENABLED 1
#define ADC_OPT_DISABLED 0

#define CELL_CH_ALL 0
#define CELL_CH_1and7 1
#define CELL_CH_2and8 2
#define CELL_CH_3and9 3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

#define SELFTEST_1 1
#define SELFTEST_2 2

#define AUX_CH_ALL 0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6

#define STAT_CH_ALL 0
#define STAT_CH_SOC 1
#define STAT_CH_ITEMP 2
#define STAT_CH_VREGA 3
#define STAT_CH_VREGD 4

#define DCP_DISABLED 0
#define DCP_ENABLED 1

#define PULL_UP_CURRENT 1
#define PULL_DOWN_CURRENT 0

#define NUM_RX_BYT 8
#define CELL 1
#define AUX 2
#define STAT 3
#define CFGR 0
#define CFGRB 4
#define CS_PIN 10

// ----------------------------------------------------------------------------
//
// Structs
//
// ----------------------------------------------------------------------------
//! Cell Voltage data structure.
typedef struct
{
  uint16_t c_codes[18];//!< Cell Voltage Codes
  uint8_t pec_match[6];//!< If a PEC error was detected during most recent read cmd
} cv;

//! AUX Reg Voltage Data
typedef struct
{
  uint16_t a_codes[9];//!< Aux Voltage Codes
  uint8_t pec_match[4];//!< If a PEC error was detected during most recent read cmd
} ax;

typedef struct
{
  uint16_t stat_codes[4];//!< A two dimensional array of the stat voltage codes.
  uint8_t flags[3]; //!< byte array that contains the uv/ov flag data
  uint8_t mux_fail[1]; //!< Mux self test status flag
  uint8_t thsd[1]; //!< Thermal shutdown status
  uint8_t pec_match[2];//!< If a PEC error was detected during most recent read cmd
} st;

typedef struct
{
  uint8_t tx_data[6];
  uint8_t rx_data[8];
  uint8_t rx_pec_match;//!< If a PEC error was detected during most recent read cmd
} ic_register;

typedef struct
{
  uint16_t pec_count;
  uint16_t cfgr_pec;
  uint16_t cell_pec[6];
  uint16_t aux_pec[4];
  uint16_t stat_pec[2];
} pec_counter;

typedef struct
{
  uint8_t cell_channels;
  uint8_t stat_channels;
  uint8_t aux_channels;
  uint8_t num_cv_reg;
  uint8_t num_gpio_reg;
  uint8_t num_stat_reg;
} register_cfg;

typedef struct
{

  ic_register config;
  ic_register configb;
  cv   cells;
  ax   aux;
  st   stat;
  ic_register  com;
  ic_register pwm;
  ic_register pwmb;
  ic_register sctrl;
  ic_register sctrlb;
  bool isospi_reverse;
  pec_counter crc_count;
  register_cfg ic_reg;
  long system_open_wire;
} cell_asic;

// ----------------------------------------------------------------------------
//
// Declarations
//
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Developed by Wisconsin Racing Firmware Team
//-----------------------------------------------------------------------------

//TODO: v this is old update for new
/**
 * Enable dcc on the ltc6811 for the given @param channel and @param group of
 * temperature sensors to read back. DO NOT ENABLE ADJECNT DISCHARGE SWITCHES
 * ON THE TEMPERATURE IC as this creates a short circuit condition. Sets all
 * other switches to closed.
 */
void ltc6811_enable_temperature_dcc(uint8_t channel, cell_asic ic[]);

uint16_t ltc6811_get_c_code(uint8_t ic_i, uint8_t c_i);
uint16_t ltc6811_get_a_code(uint8_t ic_i, uint8_t c_i);

//-----------------------------------------------------------------------------
// Ported from ltc Linduino Driver
//-----------------------------------------------------------------------------
void ltc6811_adcv(uint8_t MD, uint8_t DCP, uint8_t CH);
void ltc6811_adax(uint8_t MD, uint8_t CHG);
void ltc6811_adcvax(uint8_t MD, uint8_t DCP);
void ltc6811_adaxd(uint8_t MD, uint8_t CHG);
void ltc6811_adol(uint8_t MD, uint8_t DCP);
void ltc6811_adow(uint8_t MD, uint8_t PUP);
void ltc6811_adstat(uint8_t MD, uint8_t CHST);
void ltc6811_axst(uint8_t MD, uint8_t ST);
void ltc6811_clear_discharge(cell_asic ic[]);
void ltc6811_clraux(void);
void ltc6811_clrcell(void);
void ltc6811_clrstat(void);
void ltc6811_cvst(uint8_t MD, uint8_t ST);
void ltc6811_diagn(void);
void ltc6811_init_cfg(cell_asic ic[]);
void ltc6811_init_reg_limits(cell_asic ic[]);
uint32_t ltc6811_pollAdc(void);
int8_t ltc6811_rdaux(uint8_t reg, cell_asic ic[]);
void ltc6811_rdaux_reg(uint8_t reg, uint8_t *data);
int8_t ltc6811_rdcfg(cell_asic ic[]);
uint8_t ltc6811_rdcv(uint8_t reg, cell_asic ic[]);
void ltc6811_rdcv_reg(uint8_t reg, uint8_t * data);
int8_t ltc6811_rdstat(uint8_t reg, cell_asic ic[]);
void ltc6811_rdstat_reg(uint8_t reg, uint8_t *data);
void ltc6811_reset_crc_count(cell_asic ic[]);
int16_t ltc6811_run_cell_adc_st(uint8_t adc_reg, cell_asic ic[]);
uint16_t ltc6811_run_adc_overlap(cell_asic ic[]);
int16_t ltc6811_run_adc_redundancy_st(uint8_t adc_mode, uint8_t adc_reg, cell_asic ic[]);
uint8_t ltc6811_run_openwire(cell_asic ic[]);
void ltc6811_set_cfgr(uint8_t nIC, cell_asic ic[], bool refon, bool adcopt, bool gpio[], bool dcc[]);
void ltc6811_set_cfgr_adcopt(uint8_t nIC, cell_asic ic[], bool adcopt);
void ltc6811_set_cfgr_dis(uint8_t nIC, cell_asic ic[], bool dcc[]);
void ltc6811_set_cfgr_gpio(uint8_t nIC, cell_asic ic[], bool gpio[]);
void ltc6811_set_cfgr_ov(uint8_t nIC, cell_asic ic[], uint16_t ov);
void ltc6811_set_cfgr_refon(uint8_t nIC, cell_asic ic[], bool refon);
void ltc6811_set_discharge(int Cell, cell_asic ic[]);
void ltc6811_statst(uint8_t MD, uint8_t ST);
uint16_t ltc6811_st_lookup(uint8_t MD, uint8_t ST);
void ltc6811_wrcfg(cell_asic ic[]);
void ltc6811_check_pec(uint8_t reg, cell_asic ic[]);
int8_t parse_cells(uint8_t current_ic, uint8_t cell_reg, uint8_t cell_data[], uint16_t *cell_codes, uint8_t *ic_pec);
uint16_t pec15_calc(uint8_t len, uint8_t *data);
void wakeup_idle(void);
void wakeup_sleep(void);
void write_68(uint8_t tx_cmd[], uint8_t data[]);
int8_t read_68(uint8_t tx_cmd[], uint8_t *rx_data);
void ltc6811_handle_error(uint8_t error);
void ltc6811_handle_self_check(uint8_t error);
void ltc6811_update_test_faults(bms_t * bms, cell_asic ic[]);
void ltc6811_toggle_discharge_temp_ch(uint8_t ch, bool state, cell_asic ic[]);
void ltc6811_toggle_discharge_vltg_ch(uint8_t ch, bool state, cell_asic ic[]);
void ltc6811_set_discharge_temp(bool *dcc, bool state, cell_asic ic[]);
void ltc6811_toggle_discharge_vltg_cell(uint8_t cell, bool state, bool exclusive, cell_asic ic[]);
// uint8_t ltc6811_rdcv_voltage(uint8_t reg, cell_asic ic[]);
// uint8_t ltc6811_rdcv_temperature(uint8_t reg, cell_asic ic[]);
// uint8_t ltc6811_rdaux_temp(uint8_t reg, cell_asic ic[]);
void ltc6811_stcomm(uint8_t len);
void ltc6811_wrcomm();
void ltc6811_set_tx_data(uint8_t ic_i, uint8_t data[6]);
int8_t ltc6811_rdcomm(cell_asic ic[]);
void print_wrcomm(void);

#endif /* CODE_LTC6811_BMS_H_ */
