#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"

// Function Prototypes
void main(void);
void Init_Conditions(void);
void Init_LEDs(void);
void Carlson_StateMachine(void);

// Global Variables

volatile char slow_input_down;
unsigned char display_mode;
unsigned int test_value;
char chosen_direction;
char change;
unsigned int wheel_move;
char forward;
unsigned int Last_Time_Sequence;
unsigned int cycle_time = 0;
unsigned int time_change = 0;


void main(void){
  PM5CTL0 &= ~LOCKLPM5;

  Init_Ports();                        // Initialize Ports
  Init_Clocks();                       // Initialize Clock System
  Init_Conditions();                   // Initialize Variables and Initial Conditions
  Init_Timer_B0();                     // Initialize the timer B0 that was made
  Init_Timer_B3();                     // Initialize the timer B3 that was created
  Init_LCD();
  Init_ADC();                          // Initialize ADC
  Init_DAC();                          // Initialize DAC to (4 V)
  Init_Serial_UCA0(set_baud);          // Initialize serial com

//------------------------------------------------------------------------------
// While Loop that runs as long as the car is on.
//------------------------------------------------------------------------------

  while(ALWAYS) {
    Switch_mode();                     //Switch 1 mode call
    Switch_mode_2();                   //Switch 2 mode call
    Display_Process();
    //detect();                          //Constantly detects ADC hex values and converts BCD values to the screen
    //black_flag();                      //Throws up a black flag to detect line intersection
    IOT_State();

    if(uca1_flag && one_second){
            one_second = 0;
            strcpy(usb_TX_buf, repeat);
            UCA1IE |= UCTXIE;
        }

    if(Last_Time_Sequence != Time_Sequence){
        Last_Time_Sequence = Time_Sequence;
        cycle_time++;
        time_change = 1;
     }

    //Motor Safety if statements (prevents forward and backward being active at the same time)
    if (LEFT_FORWARD_SPEED > 0 && LEFT_REVERSE_SPEED > 0) {
        no_movement();
        P1OUT |= RED_LED;
    }

    if (RIGHT_FORWARD_SPEED > 0 && RIGHT_REVERSE_SPEED > 0) {
        no_movement();
        P1OUT |= RED_LED;
    }

  }
}





