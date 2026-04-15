/*
 * display.c
 *
 *  Created on: Feb 5, 2026
 *      Author: everettbrostedt
 *
 * -----------------------------------------------------------------------------------------------------
 * Description: This file contains the display_process function. This function updates the update_display and
 * display_changed variables based on the current values of update_display and display_changed variables.
 * -----------------------------------------------------------------------------------------------------
 */

#include  "msp430.h"
#include  "functions.h"
#include  "globals.h"
#include  "macros.h"

void Display_Process(void){
  if(update_display){
    update_display = 0;
    if(display_changed){
      display_changed = 0;
      Display_Update(0,0,0,0);
    }
  }
}

