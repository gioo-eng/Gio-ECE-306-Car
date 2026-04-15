#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include  "ports.h"
#include "timers.h"
#include "LCD.h"
#include "motors.h"
#include "esp.h"

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
volatile unsigned int baud_timer = 0;

extern volatile int menu_mode;
extern volatile int current_selection;
extern volatile int cal_entry_thumb;
extern volatile int last_thumb;
extern volatile int last_left;
extern volatile int last_right;
extern volatile int WHITE;
extern volatile int BLACK;

volatile unsigned char baud_change_pending = 0;

volatile uint8_t sw2_pressed = 0;

#pragma vector=PORT4_VECTOR
__interrupt void press_button(void) {
    if (P4IFG & SW1) {
        P4IFG &= ~SW1; 

        if (menu_mode == 0) {
            menu_mode = current_selection;
            
            cal_entry_thumb = last_thumb; 

        }
        
        else if(menu_mode == 1) {
        // --- EXITING MISSION MODE ---
        menu_mode = 0;             
        robot_state = STATE_IDLE; 
        turn_off_all();
        }
        
        else if (menu_mode == 2) {
            WHITE = (last_left + last_right) / 2; 
        }
    }
}


#pragma vector=PORT2_VECTOR
__interrupt void Port2_ISR(void) {
    if (P2IFG & SW2) {
        P2IFG &= ~SW2; 

        if (menu_mode == 2) {
            BLACK = (last_left + last_right) / 2;
        }
    }
}



#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {

    P2OUT |= IR_LED;
    Time_Sequence++;
tick_flag = 1;
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