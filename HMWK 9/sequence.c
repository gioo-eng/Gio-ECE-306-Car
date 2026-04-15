/*
 * sequence.c
 *
 *  Created on: Feb 24, 2026
 *      Author: everettbrostedt
 */

#include <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "globals.h"
#include  "macros.h"

int timer = 0;

/*
void project_5(void){
    if(one_second){
        one_second = 0;

        timer++;

        switch(timer){
            case 2://initial start
                forward_movement();
                display_mode_i = FORWARD_I;
                break;
            case 3://move for 1 second
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 4://pause for 1 second
                reverse_movement();
                display_mode_i = STOP_I;
                break;
            case 6://reverse for 2 second
                no_movement();
                display_mode_i = REVERSE_I;
                break;
            case 7://pause for 1 second
                forward_movement();
                display_mode_i = FORWARD_I;
                break;
            case 8://forward for 1 second
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 9://pause for 1 second
                turn_right();
                display_mode_i = TURN_I;
                break;
            case 12://turning 3 seconds clockwise
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 14://pause for 2 seconds
                turn_left();
                display_mode_i = TURN_I;
                break;
            case 17://turn 3 seconds counterclockwise
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 19:
                timer = 0;
                project_5_flag = 0;
                break;//pause for 2 seconds
    }
  }
}

*/

void delay (int delay_amount){   //delay amount in seconds

}

void small_delay(int small_delay_amount){       //delay for 0.1 seconds per 1 small_delay_amount
    while(small_delay_amount > 0){
            if(zero_point_one){
                zero_point_one = 0;
                small_delay_amount--;
            }
        }
}
