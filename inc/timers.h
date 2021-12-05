/*
 *
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

// ----------------------------------------------------------------------------*
//
// Defines
//
// ----------------------------------------------------------------------------*

#define TIMER_TICKS_MAX     0xffffffff
#define TIMER_TICKS_1US     80
#define TIMER_TICKS_1MS     80000
#define TIMER_TICKS_5MS     400000
#define TIMER_TICKS_10MS    800000
#define TIMER_TICKS_25MS    2000000
#define TIMER_TICKS_50MS    4000000
#define TIMER_TICKS_100MS   8000000
#define TIMER_TICKS_1S      80000000
#define TIMER_TICKS_2S      160000000
#define TIMER_TICKS_10S     800000000

#define INT_TIMER0A_PRIORITY 0x10
#define INT_TIMER1A_PRIORITY 0x10

// ----------------------------------------------------------------------------*
//
// Functions
//
// ----------------------------------------------------------------------------*
void timer0_init(void);
void timer1_init(void);

#endif /* INC_TIMERS_H_ */
