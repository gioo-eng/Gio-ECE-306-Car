/*
 * adc.c
 *
 * Created on: Mar 3, 2026
 * Author: yanny
 */

#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include  "ports.h"
#include "timers.h"
#include "LCD.h"
#include "motors.h"


  volatile int thumb = 0;
  volatile int current_channel = 2;
  volatile int left_ir = 0;
  volatile int right_ir = 0;
  volatile int condition = OFF;
  volatile int start_timed_turn = 0;

void init_adc(void) {

  ADCCTL0 = 0;
  ADCCTL0 |= ADCSHT_2; // underscore 2
  ADCCTL0 |= ADCMSC;
  ADCCTL0 |= ADCON;

  ADCCTL1 = 0;
  ADCCTL1 |= ADCSHS_0;
  ADCCTL1 |= ADCSHP;
  ADCCTL1 &= ~ADCISSH;
  ADCCTL1 |= ADCDIV_0;
  ADCCTL1 |= ADCSSEL_0;
  ADCCTL1 |= ADCCONSEQ_0;

  ADCCTL2 = 0;
  ADCCTL2 |= ADCPDIV0;
  ADCCTL2 |= ADCRES_1; // Underscore 1
  ADCCTL2 &= ~ADCDF;
  ADCCTL2 &= ~ADCSR;

  ADCMCTL0 |= ADCSREF_0;
  ADCMCTL0 |= ADCINCH_2;

  ADCIE |= ADCIE0;
  ADCCTL0 |= ADCENC;
  ADCCTL0 |= ADCSC;
}

#pragma vector=ADC_VECTOR
__interrupt void adc_interrupt(void) {
  switch(__even_in_range(ADCIV, ADCIV_ADCIFG)) {
    case ADCIV_ADCIFG:
      ADCCTL0 &= ~ADCENC;
      switch(current_channel) {
        case 2:
          left_ir = ADCMEM0;
          left_ir = left_ir >> 2;
          ADCMCTL0 &= ~ADCINCH_2;
          ADCMCTL0 |= ADCINCH_3;
          current_channel = 3;
          break;
        case 3:
          right_ir = ADCMEM0;
          right_ir = right_ir >> 2;
          ADCMCTL0 &= ~ADCINCH_3;
          ADCMCTL0 |= ADCINCH_5;
          current_channel = 5;
          break;
        case 5:
          thumb = ADCMEM0;
          thumb = thumb >> 2;
          ADCMCTL0 &= ~ADCINCH_5;
          ADCMCTL0 |= ADCINCH_2;
          current_channel = 2;
          break;
      }
          break;
  }
}
