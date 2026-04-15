/*
 * shapes.c
 *
 *  Created on: Feb 17, 2026
 *      Author: everettbrostedt
 * ---------------------------------------------------------------------------------------------------------------------------------------------------
 * Description: This file contains functions that make the circle do various shapes. The first shape is a circle, the second shape is a figure-8 and
 * the final shape is a triangle. These are the function of Project 4.
 * ---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include  "msp430.h"
#include  "functions.h"
#include  "globals.h"
#include  "ports.h"
#include  "macros.h"

unsigned int right_motor_count = 0;                 //declaration of the right motor tick counter
unsigned int left_motor_count = 0;                  //declaration of the left motor tick counter
unsigned int segment_count = 0;                     //a number that says how many "segments" should be ran
unsigned int delay_start = 0;                       //count that indicates how many chunks of time have passed before start
unsigned int figure8_count = 0;
unsigned int triangle_count = 0;
unsigned int triangle_second = 0;

//Cases that are shared between all switch cases
void wait_case(void){
    if(time_change){
        time_change = 0;
        if(delay_start++ >= WAITING2START){
            delay_start = 10;
            state = START;
        }
    }
}

void start_case(void){
    cycle_time = 0;
    right_motor_count = 0;
    left_motor_count = 0;
    forward_movement();
    segment_count = 0;
    figure8_count = 0;
    triangle_second = 0;
    triangle_count = 0;
    state = RUN;
}

void end_case(void){
    no_movement();
    state = WAIT;
    event = NONE;
    figure8_count = 0;
    triangle_count = 0;
    triangle_second = 0;
}

//run case that makes two circle shapes in a row
void circle_case(void){
    if(time_change){
        time_change = 0;
        if(segment_count <= CIRCLE_TRAVEL_DISTANCE){
            if(right_motor_count++ >= CIRCLE_RIGHT_COUNT_TIME){
                P6OUT &= ~R_FORWARD;
            }
            if(left_motor_count++ >= CIRCLE_LEFT_COUNT_TIME){
                P6OUT &= ~L_FORWARD;
            }
            if(cycle_time >= CIRCLE_WHEEL_COUNT_TIME){
                cycle_time = 0;
                right_motor_count = 0;
                left_motor_count = 0;
                segment_count++;
                forward_movement();
            }
        }else{
            state = END;
        }
    }
}

//run case that makes a circle in one direction (v1)
void one_circle_direction_1(void){
    if(time_change){
        time_change = 0;
        if(segment_count <= FIRST_TRAVEL_DISTANCE){
            if(right_motor_count++ >= FIRST_RIGHT_COUNT_TIME){
                P6OUT &= ~R_FORWARD;
            }
            if(left_motor_count++ >= FIRST_LEFT_COUNT_TIME){
                P6OUT &= ~L_FORWARD;
            }
            if(cycle_time >= FIRST_WHEEL_COUNT_TIME){
                cycle_time = 0;
                right_motor_count = 0;
                left_motor_count = 0;
                segment_count++;
                forward_movement();
            }
        }else{
            segment_count = 0;
            right_motor_count = 0;
            left_motor_count = 0;
            cycle_time = 0;
            state = THREE;
        }
    }
}

//run case that makes a circle in the other direction (v1)
void second_circle_direction_1(void){
    if(time_change){
        time_change = 0;
        if(segment_count <= SECOND_TRAVEL_DISTANCE){
            if(right_motor_count++ >= SECOND_RIGHT_COUNT_TIME){
                P6OUT &= ~R_FORWARD;
            }
            if(left_motor_count++ >= SECOND_LEFT_COUNT_TIME){
                P6OUT &= ~L_FORWARD;
            }
            if(cycle_time >= SECOND_WHEEL_COUNT_TIME){
                cycle_time = 0;
                right_motor_count = 0;
                left_motor_count = 0;
                segment_count++;
                forward_movement();
            }
        }else{
            figure8_count++;

            if(figure8_count < 2){
                segment_count = 0;
                right_motor_count = 0;
                left_motor_count = 0;
                cycle_time = 0;
                state = RUN;
            }else{
                state = END;
            }
        }
    }
}


//run case that makes a straight line for triangle leg
void straight_case(void){
    if(time_change){
        time_change = 0;
        if(segment_count <= STRAIGHT_TRAVEL_DISTANCE){
            if(right_motor_count++ >= STRAIGHT_RIGHT_COUNT_TIME){
                P6OUT &= ~R_FORWARD;
            }
            if(left_motor_count++ >= STRAIGHT_LEFT_COUNT_TIME){
                P6OUT &= ~L_FORWARD;
            }
            if(cycle_time >= STRAIGHT_WHEEL_COUNT_TIME){
                cycle_time = 0;
                right_motor_count = 0;
                left_motor_count = 0;
                segment_count++;
                no_movement();
                reverse_movement();
            }
        }else{
            segment_count = 0;
            right_motor_count = 0;
            left_motor_count = 0;
            cycle_time = 0;
            state = FOUR;
        }
    }
}

//run case that makes a 60 degree turn for triangle
void turn_case(void){
    if(time_change){
        time_change = 0;
        if(segment_count <= TURN_TRAVEL_DISTANCE){
            if(right_motor_count++ >= TURN_RIGHT_COUNT_TIME){
                P6OUT &= ~R_FORWARD;
            }
            if(left_motor_count++ >= TURN_LEFT_COUNT_TIME){
                P6OUT &= ~L_FORWARD;
            }
            if(cycle_time >= TURN_WHEEL_COUNT_TIME){
                cycle_time = 0;
                right_motor_count = 0;
                left_motor_count = 0;
                segment_count++;
                no_movement();
                forward_movement();
            }
        }else{
            triangle_count++;
                if(triangle_count < 6){
                    segment_count = 0;
                    right_motor_count = 0;
                    left_motor_count = 0;
                    cycle_time = 0;
                    state = RUN;
                }else{
                    state = END;
                }
        }

    }
}

//run case that makes the robot do a circle
void Run_Circle(void){
     switch(state){
        case WAIT: // Begin
            wait_case();
            break; //
        case START: // Begin
            start_case();
            break; //
        case RUN: // Run
            circle_case();
            break; //
        case END: // End
            end_case();
            break; //
        default: break;
     }
}

//Complete Set of cases that cause a figure 8 to run
void Run_Figure_8(void){
     switch(state){
        case WAIT: // Begin
            wait_case();
            break; //
        case START: // Begin
            start_case();
            break; //
        case RUN:
            one_circle_direction_1();
            break;
        case THREE:
            second_circle_direction_1();
            break;
        case END: // End
            end_case();
            break; //
        default: break;
     }
}

void Run_Triangle(void){
     switch(state){
        case WAIT: // Begin
            wait_case();
            break; //
        case START: // Begin
            start_case();
            break; //
        case RUN: // Run
            straight_case();
            break; //
        case FOUR:
            turn_case();
            break;
        case END: // End
            end_case();
            break; //
        default: break;
     }
}


