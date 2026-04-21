/*
 * mission.c
 *
 * Created on: Apr 15, 2026
 */

#include "msp430.h"
#include "functions.h"
#include "macros.h"
#include "motors.h"
#include "timers.h"

// Bring in the global variables needed for the state machine
extern volatile unsigned int robot_state;
extern volatile int timer_seconds;
extern volatile int left_ir;
extern volatile int right_ir;
extern volatile int start_timed_turn;
extern volatile int system_running;
extern volatile int pid_ready_flag;
extern unsigned int time_change; 
extern volatile unsigned char update_display; 
extern volatile int BLACK;
extern volatile int thumb;
volatile unsigned int pid_start_time = 0;
volatile int is_circle = 0;
volatile int countdown_start_time = 0;
void Update_Project_Display(int left_val, int right_val, int thumb);

void Run_Mission_Machine(void) {
    switch (robot_state) {
        case STATE_COUNTDOWN:
            turn_off_all();
        if ((timer_seconds - countdown_start_time) >= 2) {        
                    robot_state = STATE_DRIVE_TO_LINE;
            }
            break;

        case STATE_DRIVE_TO_LINE:
            LEFT_FORWARD_SPEED  = 27000;
            RIGHT_FORWARD_SPEED = SLOW_R;
            if (left_ir > (BLACK - 20) && right_ir > (BLACK - 20)) {
                turn_off_all();
                update_display = 1;
                robot_state = STATE_INTERCEPT;

            }
            break;

        case STATE_INTERCEPT:
            update_display = 1;
            ms_delay(5000);
            turn_off_all();
            robot_state = STATE_EXECUTE_TURN;
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
            
            while (left_ir <= 185 || right_ir <= 185) {
            }
            
            turn_off_all();
            ms_delay(4000);
            Init_PID();
            pid_start_time = timer_seconds; 
            is_circle = 0;
            robot_state = STATE_LINE_FOLLOW;
            break;

        case STATE_LINE_FOLLOW:
            if (time_change == 1) {
               if (is_circle == 0 && (timer_seconds - pid_start_time) >= 15) {
                    is_circle = 1;     
                    update_display = 1; 
                }
               
            if ((timer_seconds - pid_start_time) >= 100) {         
                           turn_off_all();
                    robot_state = STATE_EXIT;
                    break;
                }
            }
            if (pid_ready_flag == 1) {
                pid_ready_flag = 0;
                Run_PID();
            }
            break;

        case STATE_EXIT:
            Update_Project_Display(left_ir, right_ir, thumb);
            update_display = 1; 
            Display_Process();
            turn_off_all();
            RIGHT_FORWARD_SPEED = WHEEL_OFF;
            LEFT_REVERSE_SPEED  = WHEEL_OFF;
            RIGHT_REVERSE_SPEED = t_R;
            LEFT_FORWARD_SPEED  = t_L;
            ms_delay(800);
            turn_off_all();
            ms_delay(4000);
            LEFT_FORWARD_SPEED  = SLOW_R;
            RIGHT_FORWARD_SPEED = SLOW_L;
            LEFT_REVERSE_SPEED  = WHEEL_OFF;
            RIGHT_REVERSE_SPEED = WHEEL_OFF;
            ms_delay(1600);
            turn_off_all();
            robot_state = STATE_STOP;    
            break;
        
        case STATE_STOP:
            turn_off_all();
            break;

        default:
            turn_off_all();
            robot_state = STATE_STOP;
            system_running = 0; // Release the system
            break;
    }
}