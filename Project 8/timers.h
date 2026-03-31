/*
 * timers.h
 *
 *  Created on: Feb 26, 2026
 *      Author: yanny
 */

#ifndef TIMERS_H_
#define TIMERS_H_
#include <stdint.h>  

void Init_Timer_B0(void);
void Init_Timer_B1(void);
void Init_Timers(void);
void Init_Timer_B3(void);
int is_timer_running(unsigned int duration_ticks);


#define TB0CCR0_INTERVAL (25000)
#define TICKS_1S (5)
#define TICKS_15S (7)
#define TICKS_2S (10)
#define TICKS_3S (15)
#define TICKS_start (3)


extern volatile uint8_t tick_flag;




//////



extern volatile unsigned int debounce_counter;
extern volatile unsigned int is_debouncing;




#endif /* TIMERS_H_ */
