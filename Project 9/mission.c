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
extern volatile unsigned char update_display; // Assuming this is char or int based on your display.c

void Run_Mission_Machine(void) {
    switch (robot_state) {
        case STATE_COUNTDOWN:
            turn_off_all();
            if (timer_seconds >= 2) {
                robot_state = STATE_DRIVE_TO_LINE;
            }
            break;

        case STATE_DRIVE_TO_LINE:
            LEFT_FORWARD_SPEED  = SLOW_L;
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
            system_running = 0; // Release the system to process the next IoT command
            break;

        default:
            turn_off_all();
            robot_state = STATE_IDLE;
            system_running = 0; // Release the system
            break;
    }
}