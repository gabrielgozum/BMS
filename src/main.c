/**
 * @brief main for BMS project
 *
 * This file is the top level file for this project.  It executes all
 * initialization functions and provides the main loop for the program
 */

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/fpu.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/tm4c123gh6pm.h"
#include "Lib_watchdog.h"
#include "adc.h"
#include "balancing.h"
#include "bms.h"
#include "can.h"
#include "charger.h"
#include "current.h"
#include "fault_manager.h"
#include "eeprom.h"
#include "ltc1380.h"
#include "ltc6811.h"
#include "ltc1864.h"
#include "main.h"
#include "params.h"
#include "scheduler.h"
#include "sdc_control.h"
#include "spi.h"
#include "temperature.h"
#include "timers.h"
#include "voltage.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

// ltc6811 ADC Commands configuration
const uint8_t ADC_OPT = ADC_OPT_DISABLED;
const uint8_t ADC_CONVERSION_MODE = MD_7KHZ_3KHZ; //MD_26HZ_2KHZ, MD_7KHZ_3KHZ
const uint8_t ADC_DCP = DCP_ENABLED; // Discharging allowed during measurement
const uint8_t CELL_CH_TO_CONVERT = CELL_CH_ALL;
const uint8_t CELL_CH_MAX = CELL_CH_5and11;
const uint8_t AUX_CH_TO_CONVERT = AUX_CH_ALL;
const uint8_t STAT_CH_TO_CONVERT = STAT_CH_ALL;

static uint8_t channel = 0;

cell_asic bms_ic[TOTAL_ICS]; //!< stores register values sent and received from the ltc6811 ICs

scheduler_task_t scheduler_table[] =
        {
//#ifndef DEBUG
//         {watchdog_feed, 0, 0, true},  // always run
//#endif
         {can_data_handler, 0, 0, true},  // always run

         // take measurements
         {main_start_measure_voltage_all, MAIN_UPDATE_MEASUREMENTS_PRIMARY_PERIOD, 0, true}, // function, period, last called, active
         {main_start_measure_temperature_channel, 100, 0, false},

         // calculations
         {dcc_multiple_cell_balancing, 500, 0, false},
         {current_update_dcl, MAIN_UPDATE_MEASUREMENTS_PRIMARY_PERIOD, 0, false},
         {current_update_ccl, MAIN_UPDATE_MEASUREMENTS_PRIMARY_PERIOD, 0, true},
         {current_update_soc, UPDATE_SOC_PERIOD, 0, true},
         {current_enforce_limits, CL_CHECK_PERIOD, 0, true},
         {charger_set_command_values, CHARGER_COMMAND_PERIOD, 0, true},

         // send CAN messages
         {can_send_cell_broadcast_1, 10, 0, true},
         {can_send_cell_broadcast_2, 10, 0, true},
         {can_send_cell_broadcast_3, 10, 0, true},
         {can_send_pack_stats, 100, 0, true},
         {can_send_vitals, 100, 0, true},
         {can_send_charger_command, CHARGER_COMMAND_PERIOD, 0, false},
         {can_send_stats_cell_temperature, 100, 0, true},
         {can_send_stats_cell_voltage, 100, 0, true},
         {can_send_stats_ocv, 100, 0, true},
         {can_send_stats_cell_dch_res, 100, 0, true},
         {can_send_stats_cell_ch_res, 100, 0, true},
         {can_send_bms_version, 10000, 0, true},
         {can_decrement_count, 1, 0, true},
         {can_send_cell_balancing, 100, 0, true},
         {can_send_debug, 500, 0, true},
         {can_send_stats_soc, 100, 0, true},
         {can_send_stats_capacity, 100, 0, true},
         {can_send_fault_matrix, 100, 0, true}
        };


uint16_t scheduler_length = sizeof(scheduler_table) / sizeof(scheduler_task_t);


/**
 * @brief enable clocking for all GPIO ports
 */
static void main_gpio_init(void)
{
    // enable clocking
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // wait for clocking to be ready
    while ( (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
            && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
            && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
            && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
            && (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    ) {};
}


/**
 * @brief Call peripheral initializations
 */
static void main_periph_init(void)
{
    // Disable all interrupts during initialization
    IntMasterDisable();

    FPUEnable(); // need because working with floats

    main_gpio_init();
    SSI1_init();
    can_init();
    adc_adc_init();
    adc_ain_init();
    ltc1864_init();
    sdc_control_init();
    scheduler_init(TICKS_PER_SECOND);
    timer0_init();
    timer1_init();

//#ifndef DEBUG
//     watchdog_init(1000);
//#endif

    // Enable interrupts
    IntMasterEnable();
}


/**
 * @brief Initialize global variables
 */
static void main_vars_init(void)
{
    bms_init();
    ltc6811_init_cfg(bms_ic);
    ltc6811_init_reg_limits(bms_ic);
}


void main_measure_voltage_startup(void)
{
    // Wake up the ISOspi port. Returns to idle after 5.5ms.
    wakeup_sleep();
    ltc6811_adcv(ADC_CONVERSION_MODE, DCP_DISABLED, CELL_CH_ALL);
    uint32_t counter = ltc6811_pollAdc();

    // Read measurements from ic
    uint8_t error = ltc6811_rdcv(CELL_CH_ALL, bms_ic); 
    ltc6811_handle_error(error);

    uint8_t group_i;
    for (group_i = 1; group_i < 5; group_i++)
    {
        bms_transfer_cell_voltages(group_i);
    }

    bms.i_pack[0] = current_get_measurement();
}


void main_start_measure_voltage_all(void)
{
    wakeup_idle();

    // measure voltages and GPIO1
    ltc6811_adcvax(ADC_CONVERSION_MODE, DCP_DISABLED);

    current_start_measurement();

    TimerLoadSet(TIMER0_BASE, TIMER_BOTH, 3562 * TIMER_TICKS_1US);
    TimerEnable(TIMER0_BASE, TIMER_BOTH);

    scheduler_task_disable(main_start_measure_voltage_all);
}


void main_read_measure_voltage_all(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerDisable(TIMER0_BASE, TIMER_BOTH);

    // read only voltage measurements on startup
    uint8_t error = ltc6811_rdcv(CELL_CH_ALL, bms_ic);
    ltc6811_handle_error(error);

    //TODO: groups may be unnecessary
    uint8_t group_i;
    for (group_i = 1; group_i < 5; group_i++)
    {
        bms_transfer_cell_voltages(group_i);
    }

    bms.i_pack[0] = current_get_measurement();
    bms_calc_cell_stats();
    bms_check_for_voltage_faults();

    scheduler_task_enable(main_start_measure_temperature_channel, false);
}

void main_start_measure_temperature_channel(void)
{
    // update i2c muxes for temp sense
    ltc1380_send_i2c(channel);

    // read from GPIO1
    uint8_t error = 0;
    error = ltc6811_rdaux(AUX_CH_GPIO1, bms_ic); // TODO: experiment with AUX_CH_ALL? Also should be unnecessary with the adcvax command
    ltc6811_handle_error(error);

    TimerLoadSet(TIMER1_BASE, TIMER_BOTH, 2500 * TIMER_TICKS_1US);
    TimerEnable(TIMER1_BASE, TIMER_BOTH);

    scheduler_task_disable(main_start_measure_temperature_channel);
}

void main_read_temperature_channel(void)
{
   TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
   TimerDisable(TIMER1_BASE, TIMER_BOTH);

   // cell_asic data --> BMS struct
   bms_transfer_cell_temperature(channel);

    channel++;
    if (channel >= CELLS_PER_BSB)
    {
       channel = 0;
    }

    scheduler_task_enable(main_start_measure_voltage_all, false);
}


void main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // initialize hardware
    main_periph_init();

    // initialize global variables
    main_vars_init();

    //-------------------------------------------------------------------------
    // ltc6811 configuration and self check test
    //-------------------------------------------------------------------------
    wakeup_sleep();
    ltc6811_wrcfg(bms_ic);
    int16_t self_check_error = ltc6811_run_cell_adc_st(CELL, bms_ic);
    ltc6811_handle_self_check(self_check_error);

    //-------------------------------------------------------------------------
    // Initial measuremetns and estimates. Calibrate the current sensor drift
    // and then take initial voltage measurements. The cell temperature is
    // assumed to be 25 deg C and then it will be actually measured once the
    // main loop starts running and resistance values will be updated.
    //-------------------------------------------------------------------------
    SysCtlDelay(80000000 / 3); // wait 1 second
    bms.i_pack_offset = current_calc_offset();
    main_measure_voltage_startup();

    uint8_t cell_i;
    for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
    {
        float soc = current_lookup_soc(bms_get_open_cell_voltage_calc(cell_i));
        bms_set_cell_soc(cell_i, soc);
        bms_set_cell_capacity(cell_i, soc * PACK_NOMINAL_CAPACITY);
    }

    bms_calc_cell_stats();

    //-------------------------------------------------------------------------
    // Detect if the charger is connected on the CANBUS. Reconfigure the BMS
    // based on charger connection.
    //-------------------------------------------------------------------------
    uint32_t time_start = scheduler_ticks_get(); // store the time the measurement started
    while (scheduler_elapsed_ticks(time_start) < CAN_TIMEOUT_MAX && !fault_is_fChargerConnected())
    {
        can_data_handler(); // Poll the CAN bus until timed out or a message is received
    }

    if (!fault_is_fChargerConnected())
    {
        //---------------------------------------------------------------------
        // Charger not detected, reconfigure BMS
        //---------------------------------------------------------------------
        can_discharge_configure();
        bms_set_driving_limits(&(bms.limits));
        scheduler_task_disable(charger_set_command_values);
        scheduler_task_disable(can_send_charger_command);
        scheduler_task_enable(current_update_dcl, false);
    }

    // calculate current limits - leave dcl as 0 if in charging mode
    current_update_dcl();
    current_update_ccl();

    // send some initial data
    can_send_stats_cell_temperature();
    can_send_stats_cell_voltage();
    can_send_stats_ocv();
    can_send_stats_cell_dch_res();
    can_send_stats_cell_ch_res();
    can_send_vitals();
    can_send_pack_stats();

    //-------------------------------------------------------------------------
    // Setup complete, close the AIRs if ccl/dcl is not limited
    //-------------------------------------------------------------------------
    fault_set_fVehicleIdle();
    if (bms.charge_current_limit != 0 && fault_is_fChargerConnected())
    {
        // close airs because in charging mode and the ccl is nonzero
        sdc_control_close();
    }
    else if (bms.discharge_current_limit != 0 && !fault_is_fChargerConnected())
    {
        // close airs because in discharging mode and the dcl is nonzero
        sdc_control_close();
    }
    else
    {
        // AIRs couldn't be closed
        // if unable to close airs, still run loop to monitor cells
        fault_set_aStartupFailure();
    }

    //-------------------------------------------------------------------------
    // Main Loop
    // The main loop runs the scheduler.
    //-------------------------------------------------------------------------
    for(;;)
    {
        scheduler_run();
    }
}
