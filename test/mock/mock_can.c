#include <stdbool.h>
#include <stdint.h>

void can_set_dtc(void)
{

}

void can_clr_dtc(void)
{

}

void can_send_ct(void)
{

}

void can_send_bsbth(void)
{

}

void can_send_fault(void)
{

}

void can_send_cv(uint8_t cell_id, uint16_t ccv, uint16_t ocv, uint16_t cv_fault, uint16_t resistance)
{
    cell_id++;
    ccv++;
    ocv++;
    cv_fault++;
    resistance++;
}

