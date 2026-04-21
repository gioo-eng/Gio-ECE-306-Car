#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include "ports.h"

unsigned int DAC_data = 0;

void Init_DAC(void){

  SAC3DAC  = DACSREF_0;                 // Select AVCC as DAC reference
  SAC3DAC |= DACLSEL_0;                 // DAC latch loads when DACDAT written

  SAC3OA   = NMUXEN;                    // SAC Negative input MUX controL
  SAC3OA  |= PMUXEN;                    // SAC Positive input MUX control
  SAC3OA  |= PSEL_1;                    // 12-bit reference DAC source selected
  SAC3OA  |= NSEL_1;                    // Select negative pin input
  SAC3OA  |= OAPM;                      // Select low speed and low power mode
  SAC3PGA  = MSEL_1;                    // Set OA as buffer mode
  SAC3OA  |= SACEN;                     // Enable SAC
  SAC3OA  |= OAEN;                      // Enable OA

DAC_data = DAC_Begin;
SAC3DAT = DAC_data;
SAC3DAC |=  DACEN;                    // Enable DAC


P1OUT |= RED_LED;
P2OUT |= DAC_ENB;

TB0CTL |= TBIE;
TB0CTL &= ~TBIFG;

}
