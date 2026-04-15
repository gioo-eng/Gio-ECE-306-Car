/*
 * adc.c
 *
 *  Created on: Mar 1, 2026
 *      Author: everettbrostedt
 */
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "macros.h"

unsigned int ADC_Channel = CHANNEL_2;
unsigned volatile int ADC_Left_Detect;
unsigned volatile int ADC_Right_Detect;
unsigned volatile int ADC_Thumb;

volatile unsigned char left_flag = 0;
volatile unsigned char right_flag = 0;
volatile unsigned char thumb_flag = 0;

void Init_ADC(void){
//Reset the global flags for left, right and thumb
    left_flag = 0;
    right_flag = 0;
    thumb_flag = 0;

// ADCCTL0 Register
    ADCCTL0 = 0;            // Reset
    ADCCTL0 |= ADCSHT_2;    // 16 ADC clocks
    ADCCTL0 |= ADCMSC;      // MSC
    ADCCTL0 |= ADCON;       // ADC ON

// ADCCTL1 Register
    ADCCTL1 = 0;            // Reset
    ADCCTL1 |= ADCSHS_0;    // 00b = ADCSC bit
    ADCCTL1 |= ADCSHP;      // ADC sample-and-hold SAMPCON signal from sampling timer.
    ADCCTL1 &= ~ADCISSH;    // ADC invert signal sample-and-hold.
    ADCCTL1 |= ADCDIV_0;    // ADC clock divider - 000b = Divide by 1
    ADCCTL1 |= ADCSSEL_0;   // ADC clock MODCLK
    ADCCTL1 |= ADCCONSEQ_0; // ADC conversion sequence 00b = Single-channel single-conversion
// ADCCTL1 & ADCBUSY identifies a conversion is in process

// ADCCTL2 Register
    ADCCTL2 = 0;            // Reset
    ADCCTL2 |= ADCPDIV0;    // ADC pre-divider 00b = Pre-divide by 1
    ADCCTL2 |= ADCRES_2;    // ADC resolution 10b = 12 bit (14 clock cycle conversion time)
    ADCCTL2 &= ~ADCDF;      // ADC data read-back format 0b = Binary unsigned.
    ADCCTL2 &= ~ADCSR;      // ADC sampling rate 0b = ADC buffer supports up to 200 ksps

// ADCMCTL0 Register
    ADCMCTL0 |= ADCSREF_0;  // VREF - 000b = {VR+ = AVCC and VR– = AVSS }
    ADCMCTL0 |= ADCINCH_2;  // V_THUMB (0x20) Pin 2 A2
    ADCIE    |= ADCIE0;        // Enable ADC conv complete interrupt
    ADCCTL0  |= ADCENC;      // ADC enable conversion.
    ADCCTL0  |= ADCSC;       // ADC start conversion.
}

#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void){
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG)){
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG: // When a conversion result is written to the ADCMEM0
            break;// before its previous conversion result was read.
        case ADCIV_ADCTOVIFG: // ADC conversion-time overflow
            break;
        case ADCIV_ADCHIIFG: // Window comparator interrupt flags
            break;
        case ADCIV_ADCLOIFG: // Window comparator interrupt flag
            break;
        case ADCIV_ADCINIFG: // Window comparator interrupt flag
            break;
        case ADCIV_ADCIFG: // ADCMEM0 memory register with the conversion result
            ADCCTL0 &= ~ADCENC;
            switch (ADC_Channel){
                case CHANNEL_2:
                     ADC_Left_Detect = ADCMEM0;
                     ADC_Left_Detect = ADC_Left_Detect >> 2;
                     ADCMCTL0 &= ~ADCINCH_2;
                     ADCMCTL0 |= ADCINCH_3;
                     left_flag = 1;
                     ADC_Channel = CHANNEL_3;
                     break;
                case CHANNEL_3:
                    ADC_Right_Detect = ADCMEM0;
                    ADC_Right_Detect = ADC_Right_Detect >> 2;
                    ADCMCTL0 &= ~ADCINCH_3;
                    ADCMCTL0 |= ADCINCH_5;
                    right_flag = 1;
                    ADC_Channel = CHANNEL_5;
                    break;
                case CHANNEL_5:
                    ADC_Thumb = ADCMEM0;
                    ADC_Thumb = ADC_Thumb >> 2;
                    ADCMCTL0 &= ~ADCINCH_5;
                    ADCMCTL0 |= ADCINCH_2;
                    thumb_flag = 1;
                    ADC_Channel = 0;
                    ADC_Channel = CHANNEL_2;
                    break;
            }
            break;
        default:
            break;
    }
}

