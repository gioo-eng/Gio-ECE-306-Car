/*
 * timerb3.c
 *
 *  Created on: Mar 3, 2026
 *      Author: everettbrostedt
 */

#include  "msp430.h"
#include  "globals.h"
#include  "functions.h"
#include  "ports.h"
#include  "macros.h"

void Init_Timer_B3(void) {
     TB3CTL = TBSSEL__SMCLK;                // SMCLK
     TB3CTL |= MC__UP;                      // Up Mode
     TB3CTL |= TBCLR;                       // Clear TAR

     PWM_PERIOD = WHEEL_PERIOD;             // PWM Period [Set this to 50005]

     TB3CCTL1 = OUTMOD_7;                   // CCR1 reset/set
     LEFT_FORWARD_SPEED = WHEEL_OFF;        // P6.1 Left Forward PWM duty cycle

     TB3CCTL2 = OUTMOD_7;                   // CCR2 reset/set
     RIGHT_FORWARD_SPEED = WHEEL_OFF;       // P6.2 Right Forward PWM duty cycle

     TB3CCTL3 = OUTMOD_7;                   // CCR3 reset/set
     LEFT_REVERSE_SPEED = WHEEL_OFF;        // P6.3 Left Reverse PWM duty cycle

     TB3CCTL4 = OUTMOD_7;                   // CCR4 reset/set
     RIGHT_REVERSE_SPEED = WHEEL_OFF;       // P6.4 Right Reverse PWM duty cycle

     TB3CCTL5 = OUTMOD_7;                   // CCR5 reset/set
     LCD_BACKLITE_DIMING = PERCENT_20;      // P6.5 LCD_BACKLITE On Diming percent


    //------------------------------------------------------------------------------
}


