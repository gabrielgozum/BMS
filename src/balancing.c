/*
 * balancing.c
 *
 *  Created on: Jun 17, 2018
 *      Author: michael
 */

#include <stdbool.h>
#include <stdint.h>
#include "balancing.h"
#include "bms.h"
#include "fault_manager.h"
#include "ltc6811.h"
#include "params.h"

extern bms_t bms; //!< stores variables used by the bms application
extern cell_asic bms_ic[TOTAL_ICS];

bool discharge_dcc[TOTAL_ICS][12];

void dcc_single_cell_balancing(void)
{
    if (bms.cell_balancing_commanded)
    {
        uint16_t vltg_diff = bms.closed_cell_voltage_stats.max - bms.closed_cell_voltage_stats.min;
        if (bms.closed_cell_voltage_stats.max > BALANCING_VLTG_LIMIT && vltg_diff > BALANCING_VLTG_DIFF)
        {
            ltc6811_toggle_discharge_vltg_cell(bms.closed_cell_voltage_stats.max_id, true, true, bms_ic);
            bms.balancing_cell_id = bms.closed_cell_voltage_stats.max_id;
            fault_set_fBalancingActive();
        }
        else
        {
            ltc6811_toggle_discharge_vltg_cell(bms.closed_cell_voltage_stats.max_id, false, true, bms_ic); // Disable all discharge gates
            fault_clear_fBalancingActive();
        }
    }
    else
    {
        ltc6811_toggle_discharge_vltg_cell(bms.closed_cell_voltage_stats.max_id, false, true, bms_ic); // Disable all discharge gates
        fault_clear_fBalancingActive();
    }

    wakeup_idle();
    ltc6811_wrcfg(bms_ic);
}

void dcc_multiple_cell_balancing(void)
{
    bms.num_cells_balancing = 0;
    static bool first_pass = true;

    if (bms.cell_balancing_commanded)
    {
        uint8_t cell_i = 0;
        for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
        {
            uint8_t c_i;
            uint8_t ic_i;
            bms_cell_to_bsb_id_ccv(cell_i, &ic_i, &c_i);

            if (bms.cells[cell_i].closed_cell_voltage > bms.balancing_vltg_limit)
            {
                bms.cells[cell_i].balancing_active = first_pass;
                discharge_dcc[ic_i][c_i] = bms.cells[cell_i].balancing_active;

                bms.balancing_cell_id = 255; //bms.closed_cell_voltage_stats.max_id;
                fault_set_fBalancingActive();
                bms.num_cells_balancing++;

            }
            else
            {
                if (discharge_dcc[ic_i][c_i])
                {
                    bms.num_cells_balancing--;
                }

                discharge_dcc[ic_i][c_i] = false;
                bms.cells[cell_i].balancing_active = 0;
            }
        }
    }
    else
    {
        uint8_t cell_i;
        for (cell_i = 0; cell_i < TOTAL_CELLS; cell_i++)
        {
            ltc6811_toggle_discharge_vltg_cell(cell_i, false, true, bms_ic); // Disable all discharge gates
            fault_clear_fBalancingActive();
            bms.cells[cell_i].balancing_active = 0;
        }
    }

    dcc_set_discharge(bms_ic);
    first_pass = false;
}

void dcc_set_discharge(cell_asic ic[])
{
    uint8_t ic_i;

    for (ic_i = 0; ic_i < TOTAL_ICS; ic_i++)
    {
        ltc6811_set_cfgr_dis(ic_i, ic, discharge_dcc[ic_i]);
    }

    wakeup_idle();
    ltc6811_wrcfg(ic);
}

void dcc_set_all_off(cell_asic ic[])
{
    bool dcc[12] = {false, false, false, false, false, false, false, false, false, false, false, false};

    uint8_t ic_i;

    for (ic_i = 0; ic_i < TOTAL_ICS; ic_i++)
    {
        ltc6811_set_cfgr_dis(ic_i, ic, dcc);
    }

    wakeup_idle();
    ltc6811_wrcfg(ic);
}

void dcc_set_temperature(cell_asic ic[], uint8_t group_i)
{
    bool dcc_group_0[12] = {true, false, true, false, true, false, true, false, true, false, true, false};
    bool dcc_group_1[12] = {false, true, false, true, false, true, false, true, false, true, false, true};
    bool dcc[12] = {false, false, false, false, false, false, false, false, false, false, false, false};

    bool *dcc_group = (group_i == 0) ? dcc_group_0 : dcc_group_1;

    uint8_t ic_i;

    // set the temperature dcc
    for (ic_i = 0; ic_i < TOTAL_ICS; ic_i = ic_i + 2)
    {
        ltc6811_set_cfgr_dis(ic_i, ic, dcc_group);
    }

    // set discharge dcc off
    for (ic_i = 1; ic_i < TOTAL_ICS; ic_i = ic_i + 2)
    {
        ltc6811_set_cfgr_dis(ic_i, ic, dcc);
    }

    wakeup_idle();
    ltc6811_wrcfg(bms_ic);
}
