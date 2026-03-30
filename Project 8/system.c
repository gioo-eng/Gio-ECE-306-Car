//------------------------------------------------------------------------------
//
//  Description: This file contains the System Configurations
//
//  Jim Carlson
//  Jan 2016
//  Built with IAR Embedded Workbench Version: V7.3.1.3987 (6.40.1)
//------------------------------------------------------------------------------
#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include  "ports.h"
#include "timers.h"
#include "LCD.h"
#include "motors.h"
#include "serial.h"

extern volatile int condition;
extern volatile int start_timed_turn;
volatile int system_running = 0;
volatile int start_countdown = 0;
volatile int timer_minutes = 0;
volatile int timer_seconds = 0;
volatile unsigned int tick_counter = 0;
volatile unsigned int robot_state = STATE_IDLE;
void enable_interrupts(void);
extern volatile int pid_ready_flag;

// From display.c
extern volatile unsigned char baud_display_active;
extern char baud_display_line[11];

// Pending baud change — set in ISR, handled in main loop
volatile SerialBaud pending_baud = 0;
volatile unsigned char baud_change_pending = 0;

#pragma vector=PORT4_VECTOR
__interrupt void press_button(void){

    // SW1: request 115200 baud
    if(P4IFG & SW1){
        P4IFG &= ~SW1;
        pending_baud = SERIAL_BAUD_115200;
        baud_change_pending = 1;
    }

    // SW2: request 460800 baud
    if(P4IFG & SW2){
        P4IFG &= ~SW2;
        pending_baud = SERIAL_BAUD_460800;
        baud_change_pending = 1;
    }
}

// Call this from the main loop — applies any pending baud rate change safely
void Serial_BaudProcess(void)
{
    if (baud_change_pending)
    {
        baud_change_pending = 0;
        Serial_setBaud(pending_baud);

        if (pending_baud == SERIAL_BAUD_115200) {
            memcpy(baud_display_line, " 115200Hz ", 10);
        } else {
            memcpy(baud_display_line, " 460800Hz ", 10);
        }
        baud_display_active = 1;
        update_display = 1;
    }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {

    P2OUT |= IR_LED;

    Time_Sequence++;

    if (system_running) {
        tick_counter++;
        if (tick_counter >= 5) {
            tick_counter = 0;
            timer_seconds++;
            if (timer_seconds >= 60) {
                timer_seconds = 0;
                timer_minutes++;
            }
        }
    }

    update_display = 1;
    TB0CCR0 += TB0CCR0_INTERVAL;
    TB0CCTL0 &= ~CCIFG;
}

#pragma vector=TIMER1_B0_VECTOR
__interrupt void Timer_B1_ISR(void) {

    ADCCTL0 |= ADCENC;
    ADCCTL0 |= ADCSC;

    pid_ready_flag = 1;

    TB1CCR0 += 1250;
    TB1CCTL0 &= ~CCIFG;
}

void enable_interrupts(void){
    __bis_SR_register(GIE);
}