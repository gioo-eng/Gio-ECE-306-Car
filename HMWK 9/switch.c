/*
 * switch.c
 *
 *  Created on: Feb 18, 2026
 *      Author: everettbrostedt
 */

//------------------------------------------------------------------------------
// Switch 1 Created for Project 4
//------------------------------------------------------------------------------
#include <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

//Char arrays
char switch_speed_1[] = " 115,200  ";
char switch_speed_2[] = " 460,800  ";

unsigned char okay_to_look_at_switch1;
unsigned int sw1_position = 0;
unsigned int count_debounce_SW1;
unsigned int number_of_presses = 0;

unsigned char okay_to_look_at_switch2;
unsigned char sw2_position;
unsigned int count_debounce_SW2;
unsigned int number_of_presses_2 = 0;

volatile unsigned int switch_1_count = 0;
volatile unsigned int switch_2_count = 0;
unsigned int project_5_flag = 0;
unsigned int line_detection_flag = 0;
unsigned int first = 0;

unsigned int black_value_R = 0;
unsigned int black_value_L = 0;
unsigned int white_value_R = 0;
unsigned int white_value_L = 0;

int speed_flag = 0;

void Switch_mode(void){
    if(switch_1_count){
        switch_1_count = 0;
        sw1_position = 1;
    }
}

void Switch_mode_2(void){
    if(switch_2_count){
        switch_2_count = 0;
        number_of_presses_2++;



    }
}

#pragma vector= PORT4_VECTOR
__interrupt void switch1_interrupt(void) {
    if (P4IFG & SW1) {
        P4IE &= ~SW1;
        P4IFG &= ~SW1;
        TB0CCTL1 &= ~CCIFG;
        TB0CCR1 += TB0CCR1_INTERVAL;
        TB0CCTL1 |= CCIE;
        switch_1_count = 1;
 }
}

#pragma vector= PORT2_VECTOR
__interrupt void switch2_interrupt(void) {
    if (P2IFG & SW2) {
        P2IE &= ~SW2;
        P2IFG &= ~SW2;
        TB0CCTL2 &= ~CCIFG;
        TB0CCR2 += TB0CCR2_INTERVAL;
        TB0CCTL2 |= CCIE;
        switch_2_count = 1;
 }
}

/* Code for switch 2 toggle (baud rate)
 switch(number_of_presses_2){
        case 1:
            UCA0BRW = 4;
            UCA0MCTLW = 0x5551;
            UCA1BRW = 4;
            UCA1MCTLW = 0x5551;
            speed_flag = 0;
            strcpy(display_line[2], switch_speed_1);
            display_changed = 1;
        break;
        case 2:
            number_of_presses_2 = 0;
            UCA0BRW = 17;
            UCA0MCTLW = 0x4A00;
            UCA1BRW = 17;
            UCA1MCTLW = 0x4A00;
            speed_flag = 1;
            strcpy(display_line[2], switch_speed_2);
            display_changed = 1;
        break;
        default:break;
     */


/* This used to be used for de-bounce back in project 4
//Switch 1 is used for changing shape mode
void Switch1_Process(void){
    if (okay_to_look_at_switch1 && (sw1_position == RELEASED)){
      if (!(P4IN & SW1)){
          sw1_position = PRESSED;
          okay_to_look_at_switch1 = NOT_OKAY;
          count_debounce_SW1 = DEBOUNCE_RESTART;
          number_of_presses++;
      }
    }
   if (count_debounce_SW1 <= DEBOUNCE_TIME){
       count_debounce_SW1++;
   }else{
       okay_to_look_at_switch1 = OKAY;
       if (P4IN & SW1){
           sw1_position = RELEASED;
       }
    }
  }
*/



/* This used to be used for de-bounce back in project 4
//Switch 2 is used for changing the pin 3.4 mode from GPIO to CLK
void Switch2_Process(void){
    if (okay_to_look_at_switch2 && (sw2_position == RELEASED)){
      if (!(P2IN & SW2)){
          sw2_position = PRESSED;
          okay_to_look_at_switch2 = NOT_OKAY;
          count_debounce_SW2 = DEBOUNCE_RESTART;
          number_of_presses_2++;
      }
    }
   if (count_debounce_SW2 <= DEBOUNCE_TIME){
       count_debounce_SW2++;
   }else{
       okay_to_look_at_switch2 = OKAY;
       if (P2IN & SW2){
           sw2_position = RELEASED;
       }
    }
  }
*/
