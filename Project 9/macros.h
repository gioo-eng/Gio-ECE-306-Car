/*
 * macros.h
 *
 *  Created on: Feb 5, 2026
 *      Author: yanny
 */

#ifndef MACROS_H_
#define MACROS_H_

#define ALWAYS         (1)
#define ON             (1)
#define OFF            (0)


#define RESET_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1
#define TEST_PROBE           (0x01) // 0 TEST PROBE
#define TRUE                 (0x01) //




// Function Prototypes
void main(void);
void Init_Conditions(void);
void Display_Process(void);
void Init_LEDs(void);
void Carlson_StateMachine(void);

  // Global Variables
volatile char slow_input_down;
extern char display_line[4][11];
extern char *display[4];
unsigned char display_mode;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern volatile unsigned int update_display_count;
extern volatile unsigned int Time_Sequence;
unsigned int test_value;
char chosen_direction;
char change;


extern volatile unsigned int one_time;
extern volatile unsigned int is_debouncing;
extern volatile unsigned int debounce_counter;


unsigned int wheel_move;
char forward;

// mission void
void Run_Mission_Machine(void);


#endif /* MACROS_H_ */
