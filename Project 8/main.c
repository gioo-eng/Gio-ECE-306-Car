//  Jim Carlson
//  Jan 2023
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------



#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include "macros.h"
#include "timers.h"
#include "motors.h"
#include "serial.h"
extern void Serial_BaudProcess(void);
extern volatile unsigned int tick_counter;
volatile unsigned int Time_Sequence = 0;
volatile unsigned int is_debouncing = 0;
volatile unsigned int debounce_counter = 0;
extern volatile uint8_t tick_flag;
extern volatile int condition;
extern volatile int left_ir;
extern volatile int right_ir;
extern volatile int thumb;
extern volatile int current_channel;
extern volatile int start_timed_turn;
extern volatile int system_running;
extern volatile int start_countdown;
extern volatile int pid_ready_flag;
extern volatile int WHITE;
extern volatile uint8_t sw2_pressed;

void check_motor_safety(void) {
    if ((LEFT_FORWARD_SPEED > WHEEL_OFF) && (LEFT_REVERSE_SPEED > WHEEL_OFF)) {
        LEFT_FORWARD_SPEED = WHEEL_OFF;
        LEFT_REVERSE_SPEED = WHEEL_OFF;
        P1OUT |= RED_LED;
    }

    if ((RIGHT_FORWARD_SPEED > WHEEL_OFF) && (RIGHT_REVERSE_SPEED > WHEEL_OFF)) {
        RIGHT_FORWARD_SPEED = WHEEL_OFF;
        RIGHT_REVERSE_SPEED = WHEEL_OFF;
        P1OUT |= RED_LED;
    }
}

  unsigned int Last_Time_Sequence = 0;
  unsigned int cycle_time = 0;
  unsigned int time_change = 0;



//void main(void){
void main(void){
//    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

//------------------------------------------------------------------------------
// Main Program
// This is the main routine for the program. Execution of code starts here.
// The operating system is Back Ground Fore Ground.
//
//------------------------------------------------------------------------------
  PM5CTL0 &= ~LOCKLPM5;
// Disable the GPIO power-on default high-impedance mode to activate
// previously configured port settings

  Init_Ports();                        // Initialize Ports
  Init_Clocks();   
  Init_Serial();                      // Initialize Clock System
  Init_Conditions();                   // Initialize Variables and Initial Conditions
  Init_Timers();                       // Initialize Timers
  Init_LCD();
  init_adc();
  __enable_interrupt();


  // Initialize LCD
//P2OUT &= ~RESET_LCD;
  // Place the contents of what you want on the display, in between the quotes
// Limited to 10 characters per line
  
  turn_off_all();





  Time_Sequence = 0;
  strcpy(display_line[0], "          ");
  strcpy(display_line[1], "          ");
  strcpy(display_line[2], "          ");
  strcpy(display_line[3], "          ");

  display_changed = TRUE;


//------------------------------------------------------------------------------
// Beginning of the "While" Operating System
//------------------------------------------------------------------------------



  while(ALWAYS) {
    check_motor_safety();

    SerialProcess();
    Serial_BaudProcess();

    // Handle timer tick in main loop (was in ISR before)
    if (tick_flag) {
        tick_flag = 0;

        Serial_TimerTick();   // now safe - no race with SerialProcess()

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
    }

    // Update timing flags
    if (Last_Time_Sequence != Time_Sequence) {
        Last_Time_Sequence = Time_Sequence;
        cycle_time++;
        time_change = 1;
    }


if (sw2_pressed)
{
    sw2_pressed = 0;
    Serial_TransmitBuffer();
}


    if (system_running) {
        switch(robot_state) {
            case STATE_COUNTDOWN:
                turn_off_all();
                if (timer_seconds >= 2) {
                    robot_state = STATE_DRIVE_TO_LINE;
                }
                break;

            case STATE_DRIVE_TO_LINE:
                LEFT_FORWARD_SPEED = SLOW_L;
                RIGHT_FORWARD_SPEED = SLOW_R;
                if (left_ir > (100 - 30) && right_ir > (100 - 30)) {
                    turn_off_all();
                    start_timed_turn = 1;
                    update_display = 1;
                    robot_state = STATE_EXECUTE_TURN;
                }
                break;

            case STATE_EXECUTE_TURN:
                LEFT_FORWARD_SPEED  = SLOW_R;
                RIGHT_FORWARD_SPEED = SLOW_L;
                LEFT_REVERSE_SPEED  = WHEEL_OFF;
                RIGHT_REVERSE_SPEED = WHEEL_OFF;
                ms_delay(200);
                turn_off_all();
                RIGHT_FORWARD_SPEED = WHEEL_OFF;
                RIGHT_REVERSE_SPEED = t_R;
                LEFT_FORWARD_SPEED  = t_L;
                LEFT_REVERSE_SPEED  = WHEEL_OFF;
                ms_delay(600);
                turn_off_all();
                ms_delay(1200);
                start_timed_turn = 0;
                Init_PID();
                timer_seconds = 0;
                robot_state = STATE_LINE_FOLLOW;
                break;

            case STATE_LINE_FOLLOW:
                if (time_change == 1) {
                    if (timer_seconds >= 33) {
                        turn_off_all();
                        timer_seconds = 0;
                        robot_state = STATE_FORWARD_BURST;
                        break;
                    }
                }
                if (pid_ready_flag == 1) {
                    pid_ready_flag = 0;
                    Run_PID();
                }
                break;

            case STATE_FORWARD_BURST:
                RIGHT_FORWARD_SPEED = WHEEL_OFF;
                RIGHT_REVERSE_SPEED = t_R;
                LEFT_FORWARD_SPEED  = t_L;
                LEFT_REVERSE_SPEED  = WHEEL_OFF;
                ms_delay(600);
                turn_off_all();
                LEFT_FORWARD_SPEED  = SLOW_R;
                RIGHT_FORWARD_SPEED = SLOW_L;
                LEFT_REVERSE_SPEED  = WHEEL_OFF;
                RIGHT_REVERSE_SPEED = WHEEL_OFF;
                ms_delay(2400);
                turn_off_all();
                robot_state = STATE_IDLE;
                break;

            default:
                turn_off_all();
                robot_state = STATE_IDLE;
                break;
        }
    } else {
        turn_off_all();
        robot_state = STATE_IDLE;
    }

    Update_Project_Display(left_ir, right_ir, thumb);
    Display_Process();
    P3OUT ^= TEST_PROBE;
}
}