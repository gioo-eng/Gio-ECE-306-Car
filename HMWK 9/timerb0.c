/*
 * timerb0.c
 *
 *  Created on: Feb 23, 2026
 *      Author: everettbrostedt
 */

#include <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

volatile unsigned int Time_Sequence = 0;
volatile unsigned int one_second = 0;
volatile unsigned int zero_point_one = 0;
volatile unsigned char update_display;
volatile unsigned char speed_update = 0;

void Init_Timer_B0(void){
//------------------------------------------------------------------------------
// Timer B0 initialization sets up both B0_0 and B0_1 to B0_2 and OverFlow
// 8,000,000 / 8 / 8 / [1/time]
// 1,000,000 / 8 / [1/time]
// 125,000 / [1/time]
// 1/time => 1 / 8msec => 1 / 0.005 = 200
// 125,000 / 200 = 625 – This is Capture Compare Register Interval
// 8usec per clock tick
//------------------------------------------------------------------------------
 TB0CTL = TBSSEL__SMCLK; // SMCLK
 TB0CTL |= MC__CONTINUOUS; // continuous mode
 TB0CTL |= ID__8; // Divide clock by 8
 TB0CTL |= TBCLR; // Clear Count
 TB0EX0 = TBIDEX__8; // Second Divider - Divide clock by 8

 TB0CCR0 = TB0CCR0_INTERVAL; // CCR0
 TB0CCTL0 &= ~CCIFG; // Clear CCR0 interrupt flag
 TB0CCTL0 |= CCIE; // CCR0 enable interrupt

 TB0CCR1 = TB0CCR1_INTERVAL; // CCR1
 TB0CCTL1 &= ~CCIFG; // Clear CCR1 interrupt flag
 TB0CCTL1 &= ~CCIE; // CCR1 disable interrupt

 TB0CCR2 = TB0CCR2_INTERVAL; // CCR2
 TB0CCTL2 &= ~CCIFG; // Clear CCR2 interrupt flag
 TB0CCTL2 &= ~CCIE; // CCR2 disable interrupt

 TB0CTL &= ~TBIE; // Timer A0 overflow interrupt disable
 TB0CTL &= ~TBIFG; // Clear Overflow Interrupt flag
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void){
    Time_Sequence++;
    if(Time_Sequence % 20 == 0){          //display updates every 20 counts or 200ms
            update_display = 1;
        }
    if (Time_Sequence % 100 == 0){        //every 100 counts of 10ms (1sec) one_second increments
            one_second = 1;
        }
    if (Time_Sequence % 10 == 0){         //every 10 counts of 10ms or 100ms (0.1sec)
            zero_point_one = 1;
    }
    if (Time_Sequence){
        ADCCTL0 |= ADCENC;              // Enable Conversions
        ADCCTL0 |= ADCSC;               // Start next sample every 10ms
        speed_update = 1;               // Change speed everytime a conversion occurs
    }
    TB0CCR0 += TB0CCR0_INTERVAL;    //adds the interval so that
    TB0CCTL0 &= ~CCIFG;             //clear the interrupt flag so this interrupt continues to trigger
}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void){

 switch(__even_in_range(TB0IV,14)){
     case 0:
         break;
     case 2:
         TB0CCTL1 &= ~CCIE;
         P4IFG &= ~SW1;
         P4IE |= SW1;
         TB0CCR1 += TB0CCR1_INTERVAL;
         TB0CCTL1 &= ~CCIFG;
         break;
     case 4:
         TB0CCTL2 &= ~CCIE;
         P2IFG &= ~SW2;
         P2IE |= SW2;
         TB0CCR2 += TB0CCR2_INTERVAL;
         TB0CCTL2 &= ~CCIFG;
         break;
     case 14:
         DAC_data = DAC_data - 100;
         SAC3DAT = DAC_data;

         if(DAC_data <= DAC_Limit){
            DAC_data = DAC_Adjust;
            SAC3DAT = DAC_data;
            TB0CTL &= ~TBIE;
            P6OUT &= ~GRN_LED;
          }
         break;
     default: break;
 }
}



