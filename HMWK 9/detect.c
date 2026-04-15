/*
 * adc.c
 *
 *  Created on: Mar 1, 2026
 *      Author: everettbrostedt
 */

#include <stdio.h>
#include  "msp430.h"
#include  "functions.h"
#include  "globals.h"
#include  "macros.h"

char thousands = '0';
char hundreds = '0';
char tens = '0';
char ones = '0';

int delay_amount = 0;
int white_amount = 0;
int divert_amount = 0;
int small_delay_amount = 0;
int circle_time = 0;
int center_time = 0;
int forward_time = 0;

unsigned int intersect_flag = 0;
unsigned int full_turn_flag = 0;
unsigned int right_turn_flag = 0;

unsigned int segment = WAIT_CASE;
unsigned int saved_case = FIND_LINE;

volatile int left_forward = SLOW;
volatile int right_forward = SLOW;
volatile int left_reverse = WHEEL_OFF;
volatile int right_reverse = WHEEL_OFF;

int prev_error = 0;
int i_error = 0;

//Code that takes in hex values for IR detectors and converts to BCD
void BCD_Convert(unsigned int value_passed){

    thousands = '0';
    hundreds  = '0';
    tens      = '0';
    ones      = '0';

    if(value_passed >= 1000){
         value_passed -= 1000;
         thousands = '1';
    }
    while(value_passed >= 100){
        value_passed -= 100;
        hundreds++;
    }
    while(value_passed >= 10){
        value_passed -= 10;
        tens++;
    }
    while(value_passed >= 1){
        value_passed -= 1;
        ones++;
    }
}

void detect(void){
    if(condition){
        sprintf(display_line[0], "State: ON ");
        display_changed = 1;
    }else{
        sprintf(display_line[0], "State: ON ");
        display_changed = 1;
    }

    if(left_flag){
        left_flag = 0;
        BCD_Convert (ADC_Left_Detect);
        sprintf(display_line[2], "L: %c%c%c%c   ",thousands, hundreds, tens, ones);
        display_changed = 1;
    }
    if(right_flag){
        right_flag = 0;
        BCD_Convert (ADC_Right_Detect);
        sprintf(display_line[3], "R: %c%c%c%c   ",thousands, hundreds, tens, ones);
        display_changed = 1;
    }
    if(thumb_flag){
        thumb_flag = 0;
        BCD_Convert (ADC_Thumb);
        sprintf(display_line[1], "T: %c%c%c%c   ",thousands, hundreds, tens, ones);
        display_changed = 1;
    }
    Display_Process();
}

void project_7(void){
    const double Kp = 32.4;
    const double Kd = 24.3;
    const double Ki = 0.02;

    switch(segment){
        case WAIT_CASE:
            if(one_second){
                one_second = 0;
                delay_amount++;
                if(delay_amount >= 3){
                    segment = saved_case;
                    delay_amount = 0;
                }
            }
            break;

        case OVERSHOOT:
            if(zero_point_one){
                zero_point_one = 0;
                small_delay_amount++;
                if(small_delay_amount >= 2){
                    no_movement();
                    segment = WAIT_CASE;
                    small_delay_amount = 0;
                }
            }
            break;

        case FIND_LINE:
            fast_forward();
            if(intersect_flag){
                saved_case = TURN_TO_LINE;
                segment = OVERSHOOT;
            }
            break;

        case TURN_TO_LINE:
            slow_left();
            if(right_turn_flag){
                no_movement();
                slow_right();
                if(full_turn_flag){
                    no_movement();
                    segment = NAV_LINE;
                }
            }
            if(full_turn_flag){
                no_movement();
                saved_case = NAV_LINE;
                segment = WAIT_CASE;
            }
            break;

        case NAV_LINE:
            if(speed_update){
                circle_time++;
                speed_update = 0;
                int min_speed = 5000;

                int p_error = ADC_Right_Detect - ADC_Left_Detect;
                i_error += p_error;
                if(i_error > 1000) i_error = 1000;
                if(i_error < -1000) i_error = -1000;

                int d_error = p_error - prev_error;
                int correction = (int)((Kp*p_error + Kd*d_error + Ki*i_error)*(1.2*MEDIUM/SLOW));

                if(correction > 5000) correction = 5000;
                if(correction < -5000) correction = -5000;

                right_forward = MEDIUM - correction;
                left_forward = MEDIUM + correction;

                if(right_forward > WHEEL_PERIOD) right_forward = LUCID;
                if(left_forward > WHEEL_PERIOD) left_forward = LUCID;

                if(right_forward < min_speed) right_forward = min_speed;
                if(left_forward < min_speed) left_forward = min_speed;

                RIGHT_FORWARD_SPEED = right_forward;
                LEFT_FORWARD_SPEED = left_forward;
                if(circle_time == 3000){
                    no_movement();
                    saved_case = TURN_CENTER;
                    segment = WAIT_CASE;
                }
            }
            break;

        case TURN_CENTER:
            slow_left();
            if(zero_point_one){
               zero_point_one = 0;
               center_time++;
               if(center_time >= 15){
                   segment = FIND_CENTER;
                   center_time = 0;
               }
           }
            break;

        case FIND_CENTER:
            fast_forward();
            if(one_second){
               one_second = 0;
               forward_time++;
               if(forward_time >= 3){
                   no_movement();
                   segment = WAIT_CASE;
                   forward_time = 0;
               }
           }
            break;
        default: break;
    }
}

void black_flag(void){
    full_turn_flag = 0;
    right_turn_flag = 0;
    intersect_flag = 0;

    if(ADC_Right_Detect >= 700 && ADC_Left_Detect >= 780){
        intersect_flag = 1;
    }

    if(ADC_Right_Detect >= black_value_R && ADC_Left_Detect >= black_value_L){
        full_turn_flag = 1;
    }

    if(ADC_Right_Detect >= 905 && ADC_Left_Detect < 900){
        right_turn_flag = 1;
    }
}




