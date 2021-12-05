#include <stdint.h>
#include "eeprom.h"

uint32_t eeprom_read(uint32_t address)
{
    address++;
    return 0;
}

void eeprom_write(uint32_t address, uint32_t data)
{
    address++;
    data++;
}

void eeprom_read_array(uint32_t *data_out, uint32_t address, uint32_t length)
{
    data_out++;
    address++;
    length++;
}

void eeprom_stats_load(eeprom_stats *stats)
{
	stats++; // Prevent unused param warning
}
