/*
 * balancing.h
 *
 *  Created on: Jun 17, 2018
 *      Author: michael
 */

#ifndef INC_BALANCING_H_
#define INC_BALANCING_H_

#include "ltc6811.h"

typedef struct
{
    uint8_t cell_to_balance;
    uint8_t balancing_commanded;
} balancing_t;

extern balancing_t balancing;


void dcc_multiple_cell_balancing(void);
void dcc_single_cell_balancing(void);
void dcc_set_discharge(cell_asic ic[]);
void dcc_set_temperature(cell_asic ic[], uint8_t group_i);
void dcc_set_all_off(cell_asic ic[]);


#endif /* INC_BALANCING_H_ */
