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
volatile int sw1_mode = 0;
volatile unsigned char baud_display_active = 0;
char baud_display_line[11] = "          ";
extern SerialBaud serialBaud;
volatile unsigned int baud_timer = 0;

// Pending baud change — set in ISR, handled in main loop
volatile SerialBaud pending_baud = 0;
volatile unsigned char baud_change_pending = 0;

// SW2 transmit flag — set in ISR, handled in main loop
volatile uint8_t sw2_pressed = 0;

#pragma vector=PORT2_VECTOR
__interrupt void Port2_ISR(void)
{
    if (P2IFG & SW2)
    {
        P2IFG &= ~SW2;      // clear interrupt flag
        sw2_pressed = 1;    // set flag, handled in main loop
    }
}


#pragma vector=PORT4_VECTOR
__interrupt void press_button(void){

    // SW1: request 115200 baud
    if(P4IFG & SW1){
            P4IFG &= ~SW1; // Clear the interrupt flag

            // Check the current baud rate and swap it
            if (serialBaud == SERIAL_BAUD_115200) {
                pending_baud = SERIAL_BAUD_460800;
            } else {
                pending_baud = SERIAL_BAUD_115200;
            }

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

        baud_display_active = 1; // Locks the display on
        update_display = 1;
    }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {

    P2OUT |= IR_LED;
    Time_Sequence++;
tick_flag = 1;
   Serial_TimerTick();
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
