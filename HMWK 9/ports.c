/*
 * ports.c
 *
 *  Created on: Feb 5, 2026
 *      Author: everettbrostedt
 *------------------------------------------------------------------------
 *  Description: This file contains port definitions based on the MSP430FR2355
 *  class schematic and the MSP430FR family data sheet.
 *------------------------------------------------------------------------
 */

#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"

volatile int mode = USE_GPIO;
volatile int condition = OFF;

void Init_Ports(void){
    Init_Port_1();
    Init_Port_2();
    Init_Port_3();
    Init_Port_4();
    Init_Port_5();
    Init_Port_6();
}

void Init_Port_1(void){
    P1OUT = 0x00;           // P1 set Low
    P1DIR = 0x00;           // Set P1 direction to output

    P1SEL0 &= ~RED_LED;     // RED_LED GPIO operation
    P1SEL1 &= ~RED_LED;     // RED_LED GPIO operation
    P1OUT &= ~RED_LED;      // Initial Value = Low / Off
    P1DIR |= RED_LED;       // Direction = output

    P1SEL0 |= A1_SEEED;     //A1 ADC Function
    P1SEL1 |= A1_SEEED;     //A1 ADC Function

    P1SELC |= V_DETECT_L;   //V_DETECT_L ADC Function

    P1SELC |= V_DETECT_R;   //V_DETECT_R ADC Function

    P1SEL0 |= A4_SEEED;     //A4 ADC Function
    P1SEL1 |= A4_SEEED;     //A4 ADC Function

    P1SELC |= V_THUMB;      //V_THUMB ADC Function

    P1SEL0 |= UCA0RXD;      //UCA0RXD Function
    P1SEL1 &= ~UCA0RXD;     //UCA0RXD Function

    P1SEL0 |= UCA0TXD;      //UCA0TXD Function
    P1SEL1 &= ~UCA0TXD;     //UCA0TXD Function
}

void Init_Port_2(void){
     P2OUT = 0x00; // P2 set Low
     P2DIR = 0x00; // Set P2 direction to output

     P2SEL0 &= ~SLOW_CLK; // SLOW_CLK GPIO operation
     P2SEL1 &= ~SLOW_CLK; // SLOW_CLK GPIO operation
     P2OUT &= ~SLOW_CLK; // Initial Value = Low / Off
     P2DIR |= SLOW_CLK; // Direction = output

     P2SEL0 &= ~CHECK_BAT; // CHECK_BAT GPIO operation
     P2SEL1 &= ~CHECK_BAT; // CHECK_BAT GPIO operation
     P2OUT &= ~CHECK_BAT; // Initial Value = Low / Off
     P2DIR |= CHECK_BAT; // Direction = output

     P2SEL0 &= ~IR_LED; // P2_2 GPIO operation
     P2SEL1 &= ~IR_LED; // P2_2 GPIO operation
     P2OUT |= IR_LED; // Initial Value = High / On
     P2DIR |= IR_LED; // Direction = output

     P2IE  &= ~SW2;  //interrupt is disabled to prevent accidental SW2 fires
     P2SEL0 &= ~SW2; // SW2 Operation
     P2SEL1 &= ~SW2; // SW2 Operation

     P2OUT |= SW2; // Configure pullup resistor
     P2DIR &= ~SW2; // Direction = input
     P2REN |= SW2; // Enable pullup resistor
     P2IES |= SW2; // SW2 Hi/Lo edge interrupt
     P2IFG &= ~SW2; // IFG SW2 cleared
     P2IE |= SW2; // SW2 interrupt Enabled

     P2SEL0 &= ~IOT_RUN_RED; // IOT_RUN_CPU GPIO operation
     P2SEL1 &= ~IOT_RUN_RED; // IOT_RUN_CPU GPIO operation
     P2OUT &= ~IOT_RUN_RED; // Initial Value = Low / Off
     P2DIR |= IOT_RUN_RED; // Direction = output

     P2SEL0 &= ~DAC_ENB; // DAC_ENB GPIO operation
     P2SEL1 &= ~DAC_ENB; // DAC_ENB GPIO operation
     P2OUT |= DAC_ENB; // Initial Value = High
     P2DIR |= DAC_ENB; // Direction = output

     P2SEL0 &= ~LFXOUT; // LFXOUT Clock operation
     P2SEL1 |= LFXOUT; // LFXOUT Clock operation

     P2SEL0 &= ~LFXIN; // LFXIN Clock operation
     P2SEL1 |= LFXIN; // LFXIN Clock operation
}

void Init_Port_3(void){
    P3OUT = 0x00;               // P3 set Low
    P3DIR = 0x00;               // Set P3 direction to output

    P3SEL0 &= ~TEST_PROBE;      //TEST_PROBE GPIO operation
    P3SEL1 &= ~TEST_PROBE;      //TEST_PROBE GPIO operation
    P3OUT &= ~TEST_PROBE;       // Initial Value = Low / Off
    P3DIR &= ~TEST_PROBE;       // Direction = input

    P3SEL0 &= ~OA2O;            //OA20 GPIO operation
    P3SEL1 &= ~OA2O;            //OA20 GPIO operation
    P3OUT &= ~OA2O;             // Initial Value = Low / Off
    P3DIR |= OA2O;              // Direction = output

    P3SEL0 &= ~OA2N;            //OA2N GPIO operation
    P3SEL1 &= ~OA2N;            //OA2N GPIO operation
    P3OUT &= ~OA2N;             // Initial Value = Low / Off
    P3DIR |= OA2N;              // Direction = output

    P3SEL0 &= ~OA2P;            //OA2P GPIO operation
    P3SEL1 &= ~OA2P;            //OA2P GPIO operation
    P3OUT &= ~OA2P;             // Initial Value = Low / Off
    P3DIR |= OA2P;              // Direction = output

    switch(mode){
        case USE_GPIO:
            P3SEL0 &= ~SMCLK;           //P3.4 GPIO operation
            P3SEL1 &= ~SMCLK;           //P3.4 GPIO operation
            P3OUT |= SMCLK;             // Initial Value = High/on
            P3DIR |= SMCLK;             // Direction = output
            break;
        case USE_SMCLK:
            P3SEL0 |= SMCLK;            //P3.4 SMCLK mode
            P3SEL1 &= ~SMCLK;           //P3.4 SMCLK mode
            P3DIR |= SMCLK;             //P3.4 SMCLK direction
            break;
        default: break;
    }

    P3SELC |= DAC_CNTL;         //DAC_CNTL OA3O Function (DAC Operation)

    P3SEL0 &= ~IOT_LINK_GRN;    //IOT_LINK_GRN GPIO operation
    P3SEL1 &= ~IOT_LINK_GRN;    //IOT_LINK_GRN GPIO operation
    P3OUT &= ~IOT_LINK_GRN;     // Initial Value = Low / Off
    P3DIR |= IOT_LINK_GRN;      // Direction = output

    P3SEL0 &= ~IOT_EN;          //IOT_EN GPIO operation
    P3SEL1 &= ~IOT_EN;          //IOT_EN GPIO operation
    P3OUT |= IOT_EN;           // Initial Value = HIGH/ ON
    P3DIR |= IOT_EN;            // Direction = output

}

void Init_Port_4(void){
     P4OUT = 0x00; // P4 set Low
     P4DIR = 0x00; // Set P4 direction to output

     P4SEL0 &= ~RESET_LCD; // RESET_LCD GPIO operation
     P4SEL1 &= ~RESET_LCD; // RESET_LCD GPIO operation
     P4OUT &= ~RESET_LCD; // Initial Value = Low / Off
     P4DIR |= RESET_LCD; // Direction = output

     P4SEL0 &= ~SW1; // SW1 GPIO operation
     P4SEL1 &= ~SW1; // SW1 GPIO operation
     P4OUT |= SW1; // Configure pullup resistor
     P4DIR &= ~SW1; // Direction = input
     P4REN |= SW1; // Enable pullup resistor
     P4IES |= SW1; // SW1 Hi/Lo edge interrupt
     P4IFG &= ~SW1; // IFG SW1 cleared
     P4IE |= SW1; // SW1 interrupt Enabled

     P4SEL0 |= UCA1TXD; // USCI_A1 UART operation
     P4SEL1 &= ~UCA1TXD; // USCI_A1 UART operation

     P4SEL0 |= UCA1RXD; // USCI_A1 UART operation
     P4SEL1 &= ~UCA1RXD; // USCI_A1 UART operation

     P4SEL0 &= ~UCB1_CS_LCD; // UCB1_CS_LCD GPIO operation
     P4SEL1 &= ~UCB1_CS_LCD; // UCB1_CS_LCD GPIO operation
     P4OUT |= UCB1_CS_LCD; // Set SPI_CS_LCD Off [High]
     P4DIR |= UCB1_CS_LCD; // Set SPI_CS_LCD direction to output

     P4SEL0 |= UCB1CLK; // UCB1CLK SPI BUS operation
     P4SEL1 &= ~UCB1CLK; // UCB1CLK SPI BUS operation

     P4SEL0 |= UCB1SIMO; // UCB1SIMO SPI BUS operation
     P4SEL1 &= ~UCB1SIMO; // UCB1SIMO SPI BUS operation

     P4SEL0 |= UCB1SOMI; // UCB1SOMI SPI BUS operation
     P4SEL1 &= ~UCB1SOMI; // UCB1SOMI SPI BUS operation
}

void Init_Port_5(void){
    P5OUT = 0x00;               // P5 set Low
    P5DIR = 0x00;               // Set P5 direction to output

    P5SEL0 |= V_BAT;            //V_BAT ADC Function
    P5SEL1 |= V_BAT;            //V_BAT ADC Function

    P5SEL0 |= V_5_0;            //V_5 ADC Function
    P5SEL1 |= V_5_0;            //V_5 ADC Function

    P5SEL0 |= V_DAC;            //V_DAC ADC Function
    P5SEL1 |= V_DAC;            //V_DAC ADC Function

    P5SEL0 |= V_3_3;            //V3_3 ADC Function
    P5SEL1 |= V_3_3;            //V3_3 ADC Function

    P5SEL0 &= ~IOT_BOOT;        //IOT_BOOT GPIO Function
    P5SEL1 &= ~IOT_BOOT;        //IOT_BOOT GPIO Function
    P5OUT |= IOT_BOOT;          // Initial Value = HIGH/ON
    P5DIR |= IOT_BOOT;          // Direction = output
}

void Init_Port_6(void){
    P6OUT = 0x00;                //P6 set Low
    P6DIR = 0x00;                // Set P6 direction to output

    P6SEL0 |=  LCD_BACKLITE;     //TB3.1 mode LCD_BACKLITE
    P6SEL1 &= ~LCD_BACKLITE;     //TB3.1 mode LCD_BACKLITE
    P6OUT &= ~LCD_BACKLITE;      // Initial Value = low/off
    P6DIR |= LCD_BACKLITE;       // Direction = output

    P6SEL0 |=  R_FORWARD;       //TB3.1 mode R_FORWARD
    P6SEL1 &= ~R_FORWARD;       //TB3.1 mode R_FORWARD
    P6OUT &= ~R_FORWARD;        //Set the port pin to off
    P6DIR |= R_FORWARD;         //Set the direction to output

    P6SEL0 |=  L_FORWARD;       //TB3.1 mode L_FORWARD
    P6SEL1 &= ~L_FORWARD;       //TB3.1 mode L_FORWARD
    P6OUT &= ~L_FORWARD;        //Set the port pin to off
    P6DIR |= L_FORWARD;         //Set the direction to output

    P6SEL0 |=  R_REVERSE;       //TB3.1 mode R_REVERSE
    P6SEL1 &= ~R_REVERSE;       //TB3.1 mode R_REVERSE
    P6OUT &= ~R_REVERSE;        //Set the port pin to off
    P6DIR |= R_REVERSE;         //Set the direction to output

    P6SEL0 |=  L_REVERSE;       //TB3.1 mode L_REVERSE
    P6SEL1 &= ~L_REVERSE;       //TB3.1 mode L_REVERSE
    P6OUT |= L_REVERSE;         //Set the port pin to off
    P6DIR |= L_REVERSE;         //Set the direction to output

    P6SEL0 &= ~P6_5;            //P6_5 GPIO Function
    P6SEL1 &= ~P6_5;            //P6_5 GPIO Function
    P6OUT &= ~P6_5;             // Initial Value = Low / Off
    P6DIR |= P6_5;              // Direction = output

    P6SEL0 &= ~GRN_LED;         //GRN_LED GPIO Function
    P6SEL1 &= ~GRN_LED;         //GRN_LED GPIO Function
    P6OUT &= ~GRN_LED;          // Initial Value = Low / Off
    P6DIR |= GRN_LED;           // Direction = output
}
