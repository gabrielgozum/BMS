/**
 * @brief BMS Voltages
 *
 * This file includes functions to process temperature data for cell
 * voltages.
 */

#include <stdbool.h>
#include <stdint.h>
#include "bms.h"
#include "can.h"
#include "params.h"
#include "voltage.h"


uint16_t voltage_lookup_soc(float soc)
{
    return (uint16_t)(lut_get_1d(&OCV_LUT, soc) * 10000);
}
