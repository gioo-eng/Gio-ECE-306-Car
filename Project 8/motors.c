/*
 * motors.c
 *
 *  Created on: Feb 26, 2026
 *      Author: yanny
 */
#include "functions.h"
#include "macros.h"
#include "ports.h"
#include "msp430.h"
#include "motors.h"

void turn_off_all (void){
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
}



void both_forward (void) {
    turn_off_all();
    P6OUT |= R_FORWARD;
    P6OUT |= L_FORWARD;
}

void both_reverse (void){
    turn_off_all();
    P6OUT |= R_REVERSE;
    P6OUT |= L_REVERSE;
}

void spin_cw(void){
    turn_off_all();
    P6OUT |= L_FORWARD;
    P6OUT |= R_REVERSE;
}

void spin_ccw(void){
    turn_off_all();
    P6OUT |= R_FORWARD;
    P6OUT |= L_REVERSE;
}








