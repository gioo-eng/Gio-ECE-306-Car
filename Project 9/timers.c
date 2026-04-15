/*
 * timers.c
 *
 * Created on: Feb 26, 2026
 * Author: yanny
 */

#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include "macros.h"
#include "timers.h"
#include "motors.h"

volatile int pid_ready_flag = 0;


void Init_Timers(void) {
    Init_Timer_B0();
    Init_Timer_B1();
    Init_Timer_B3();

}

volatile uint8_t tick_flag = 0;

void Init_Timer_B0(void) {
    //--------------------------------------------------------------------------
    // Timer B0 Configuration:
    // SMCLK (8MHz) / 8 (ID) / 8 (TBIDEX) = 125,000 Hz
    // 1 / 125,000 = 8usec per tick
    //--------------------------------------------------------------------------

    TB0CTL = TBSSEL__SMCLK;
    TB0CTL |= MC__CONTINUOUS;
    TB0CTL |= ID__8;
    TB0EX0 = TBIDEX__8;
    TB0CTL |= TBCLR;

    Time_Sequence = 0;

    TB0CCR0 = TB0CCR0_INTERVAL;
    TB0CCTL0 |= CCIE;

        TB0CCTL0 &= ~CCIFG;
        TB0CCTL1 &= ~CCIFG;
        TB0CCTL2 &= ~CCIFG;
        TB0CTL &= ~TBIFG;
}


void Init_Timer_B1(void) {
    TB1CTL = TBSSEL__SMCLK;
    TB1CTL |= MC__CONTINUOUS;
    TB1CTL |= ID__8;
    TB1EX0 = TBIDEX__8;
    TB1CTL |= TBCLR;

    TB1CCR0 = 1250;
    TB1CCTL0 |= CCIE;

    TB1CCTL0 &= ~CCIFG;
    TB1CTL &= ~TBIFG;
}

void Init_Timer_B3(void) {
//------------------------------------------------------------------------------
// SMCLK source, up count mode, PWM Right Side
// TB3.1 P6.0 LCD_BACKLITE
// TB3.2 P6.1 R_FORWARD
// TB3.3 P6.2 R_REVERSE
// TB3.4 P6.3 L_FORWARD
// TB3.5 P6.4 L_REVERSE
//------------------------------------------------------------------------------


TB3CTL = TBSSEL__SMCLK; // SMCLK
TB3CTL |= MC__UP; // Up Mode
TB3CTL |= TBCLR; // Clear TAR
PWM_PERIOD = WHEEL_PERIOD; // PWM Period [Set this to 50005]
TB3CCTL1 = OUTMOD_7; // CCR1 reset/set
LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle
TB3CCTL2 = OUTMOD_7; // CCR2 reset/set
RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle
TB3CCTL3 = OUTMOD_7; // CCR3 reset/set
LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.2 Left Forward PWM duty cycle
TB3CCTL4 = OUTMOD_7; // CCR4 reset/set
RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.3 Right Reverse PWM duty cycle
TB3CCTL5 = OUTMOD_7; // CCR5 reset/set
LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle
//------------------------------------------------------------------------------
}




void ms_delay(unsigned int ms) {
    unsigned int start_time = Time_Sequence;
    unsigned int duration = ms / 200;

    while ((Time_Sequence - start_time) < duration) {

        P3OUT ^= TEST_PROBE;
    }
}















