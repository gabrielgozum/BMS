#include <stdbool.h>
#include <stdint.h>
#include "ltc1380.h"
#include "ltc6811.h"

extern cell_asic bms_ic[];

static previous_channel = 0;

uint16_t ltc1380_set_channel(uint8_t channel, bool disabled)
{
    // 10010 a1 a0 0
    uint8_t address;
    // xxxx 1 c2 c1 c0
    uint8_t cmd;
    if(channel > 7)
    {
        address = 0x92;  //10010010
        cmd = (0xf << 4) | (0x8 + (0x7 & (channel - 8)));
    }
    else
    {
        address = 0x90;  //10010000
        cmd = (0xf << 4) | (0x8 + (0x7 & channel));
    }

    uint8_t disable_comp = 0;
    if (disabled)
    {
        disable_comp = 0x8;
    }

    return ((address << 8) | ((cmd & 0xff) - disable_comp));
}

void ltc1380_ltc6811_transmit_i2c(uint8_t* transmit)
{
    uint8_t ic_i = 0;

    // shift data into comm register
    for (ic_i = 0; ic_i < TOTAL_ICS; ic_i++)
    {
        ltc6811_set_tx_data(ic_i, transmit);
    }
    wakeup_sleep();

    // transfer data from bms struct to COMM registers
    ltc6811_wrcomm();

    wakeup_idle();
    // send out data over I2C
    ltc6811_stcomm(2); // data length in bytes

    //TODO: handle these errors separately
    wakeup_idle();
    int8_t error = ltc6811_rdcomm(bms_ic);
    ltc6811_handle_error(error);
}

void ltc1380_format_transmit_i2c(uint8_t channel, bool disabled)
{
    uint16_t msg = ltc1380_set_channel(channel, disabled);
    uint8_t address = (msg >> 8) & 0xff;
    uint8_t cmd = msg & 0xff;
    uint8_t transmit[6];

    // structure i2c message
    transmit[0] = ((LTC1380_START & 0xf) << 4) | ((address >> 4) & 0xf);
    transmit[1] = ((address & 0xf) << 4) | (LTC1380_NACK & 0xf);
    transmit[2] = ((LTC1380_BLANK & 0xf) << 4) | ((cmd >> 4) & 0xf);
    transmit[3] = ((cmd & 0xf) << 4) | (LTC1380_NACKSTOP & 0xf);
    transmit[4] = ((LTC1380_NOTRANSMIT & 0xf) << 4) | 0xf;
    transmit[5] = ((0xf) << 4) | (LTC1380_NACKSTOP & 0xf);

    ltc1380_ltc6811_transmit_i2c(transmit);
}

void ltc1380_send_i2c(uint8_t channel)
{
    uint16_t i = 0;
    if (previous_channel == 7)
    {
        ltc1380_format_transmit_i2c(0, true);
    }
    else if (previous_channel == 10)
    {
        ltc1380_format_transmit_i2c(8, true);
    }

    ltc1380_format_transmit_i2c(channel, false);

    previous_channel = channel;
}