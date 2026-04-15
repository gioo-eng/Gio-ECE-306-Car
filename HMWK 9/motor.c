/*
 * motor.c
 *
 *  Created on: Feb 8, 2026
 *      Author: everettbrostedt
 * ---------------------------------------------------------------------------------------------------------------------------------------------------
 * Description: This file contains a function that turns off all the motors, turns them on, and then puts them in reverse. This
 * fulfills the simple motor control described in Project 03.
 * ---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include  "msp430.h"
#include  "functions.h"
#include  "globals.h"
#include  "macros.h"

//Conditions added not only to turn off movement but also to ensure correct conditions
void forward_off(void){
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
}

void reverse_off(void){
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
}

void left_safety(void){
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
}

void right_safety(void){
    LEFT_REVERSE_SPEED = WHEEL_OFF;
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
}

void no_movement(void){
    forward_off();
    reverse_off();
}
//------------------------------------------
//   slow speed for forwards, backwards, left and right
//------------------------------------------
void slow_forward(void){
    reverse_off();
    forward_off();
    LEFT_FORWARD_SPEED = SLOW;
    RIGHT_FORWARD_SPEED = SLOW;
}

void slow_reverse(void){
    reverse_off();
    forward_off();
    RIGHT_REVERSE_SPEED = SLOW;
    LEFT_REVERSE_SPEED = SLOW;
}

void slow_left(void){
    left_safety();
    right_safety();
    RIGHT_FORWARD_SPEED = SLOW;
    LEFT_REVERSE_SPEED = SLOW;
}

void slow_right(void){
    left_safety();
    right_safety();
    LEFT_FORWARD_SPEED = SLOW;
    RIGHT_REVERSE_SPEED = SLOW;
}

//------------------------------------------

//------------------------------------------
//   medium speed for forwards, backwards, left and right
//------------------------------------------
void medium_forward(void){
    reverse_off();
    forward_off();
    RIGHT_FORWARD_SPEED = MEDIUM;
    LEFT_FORWARD_SPEED = MEDIUM;
}

void medium_reverse(void){
    reverse_off();
    forward_off();
    RIGHT_REVERSE_SPEED = MEDIUM;
    LEFT_REVERSE_SPEED = MEDIUM;
}

void medium_left(void){
    left_safety();
    right_safety();
    RIGHT_FORWARD_SPEED = MEDIUM;
    LEFT_REVERSE_SPEED = MEDIUM;
}

void medium_right(void){
    left_safety();
    right_safety();
    LEFT_FORWARD_SPEED = MEDIUM;
    RIGHT_REVERSE_SPEED = MEDIUM;
}
//------------------------------------------

//------------------------------------------
//   fast speed for forwards, backwards, left and right
//------------------------------------------
void fast_forward(void){
    reverse_off();
    forward_off();
    RIGHT_FORWARD_SPEED = FAST;
    LEFT_FORWARD_SPEED = FAST;
}

void fast_reverse(void){
    reverse_off();
    forward_off();
    RIGHT_REVERSE_SPEED = FAST;
    LEFT_REVERSE_SPEED = FAST;
}

void fast_left(void){
    left_safety();
    right_safety();
    RIGHT_FORWARD_SPEED = FAST;
    LEFT_REVERSE_SPEED = FAST;
}

void fast_right(void){
    left_safety();
    right_safety();
    LEFT_FORWARD_SPEED = FAST;
    RIGHT_REVERSE_SPEED = FAST;
}
//------------------------------------------

//------------------------------------------
//   max speed for forwards, backwards, left and right
//------------------------------------------
void lucid_forward(void){
    reverse_off();
    forward_off();
    RIGHT_FORWARD_SPEED = LUCID;
    LEFT_FORWARD_SPEED = LUCID;
}

void lucid_reverse(void){
    reverse_off();
    forward_off();
    RIGHT_REVERSE_SPEED = LUCID;
    LEFT_REVERSE_SPEED = LUCID;
}

void lucid_left(void){
    left_safety();
    right_safety();
    RIGHT_FORWARD_SPEED = LUCID;
    LEFT_REVERSE_SPEED = LUCID;
}

void lucid_right(void){
    left_safety();
    right_safety();
    LEFT_FORWARD_SPEED = LUCID;
    RIGHT_REVERSE_SPEED = LUCID;
}
//------------------------------------------







