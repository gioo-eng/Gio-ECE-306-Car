/*
 * display.c
 *
 * Created on: Feb 6, 2026
 * Author: yanny
 */

#include "msp430.h"
#include "functions.h"
#include "LCD.h"
#include "macros.h"
#include <string.h>
#include <stdio.h>
#include "esp.h"    // Needed for SSID and IP macros
#include "serial.h" // Needed for uart_get_last_frame()
#include "motors.h"

extern volatile int timer_seconds;
extern volatile unsigned int robot_state;

// --- State Machine Definitions ---
#define MAIN      0
#define MISSION   1
#define CALIBRATE 2
#define IOT       3

// --- Global Variables for Menu System ---
volatile int menu_mode = MAIN;      // Replaces 'ms' to keep compatibility with other files
volatile int current_selection = 1; // 1 = Mission, 2 = IR Cal, 3 = IOT
volatile int WHITE = 0;             
volatile int BLACK = 0;             
char last_cmd_display[11] = "WAITING   ";

// Variables for ISR access
volatile int last_thumb = 0;
volatile int cal_entry_thumb = 0;
volatile int last_left = 0;
volatile int last_right = 0;
extern volatile int is_circle;
extern volatile int iot_course_started;
extern volatile int iot_pad_state;
volatile char         last_iot_dir_char  = ' ';  
volatile unsigned int last_iot_time_units = 0U;


// --- Function Prototypes ---
void display_menu_main(int thumb);
void display_menu_mission(int left_val, int right_val, int thumb);
void display_menu_calibrate(int left_val, int right_val, int thumb);
void display_menu_iot(void);


//------------------------------------------------------------------------------
// Navigation & State Controls (Call these from your Switch ISRs)
//------------------------------------------------------------------------------
void menu_enter(void) {
    if (menu_mode == MAIN) {
        if (current_selection == 1) {
            menu_mode = MISSION;
            cal_entry_thumb = last_thumb;
        } else if (current_selection == 2) {
            menu_mode = CALIBRATE;
            cal_entry_thumb = last_thumb; // Save thumb value for twist-to-exit
        } else if (current_selection == 3) {
            menu_mode = IOT;
            cal_entry_thumb = last_thumb; // Save thumb value for twist-to-exit
        }
    }
}

void menu_back(void) {
    if (menu_mode != MAIN) {
        menu_mode = MAIN;
        
        // Optional safety: Stop the car when exiting a mode
        robot_state = STATE_IDLE;
        turn_off_all();
    }
}


//------------------------------------------------------------------------------
// Core Display Processor (Called by Main Loop)
//------------------------------------------------------------------------------
void Update_Project_Display(int left_val, int right_val, int thumb) {
    last_thumb = thumb;
    last_left = left_val;
    last_right = right_val;

    if (menu_mode == MISSION || menu_mode == CALIBRATE || menu_mode == IOT) {
        if (thumb > (cal_entry_thumb + 3) || thumb < (cal_entry_thumb - 3)) {
            menu_back(); 
        }
    }

    // --- STATE MACHINE ---
    switch (menu_mode) {
        case MAIN:
            display_menu_main(thumb);
            break;
        case MISSION:
            display_menu_mission(left_val, right_val, thumb);
            break;
        case CALIBRATE:
            display_menu_calibrate(left_val, right_val, thumb);
            break;
        case IOT:
            display_menu_iot();
            break;
    }

    display_changed = 1;
}


//------------------------------------------------------------------------------
// Individual Menu Formatters
//------------------------------------------------------------------------------

void display_menu_main(int thumb) {
    
    if (thumb < 85) {
        current_selection = 1; // MISSION
    } else if (thumb < 170) {
        current_selection = 2; // CALIBRATE
    } else {
        current_selection = 3; // IOT MODE
    }

    // 2. Format Screen Header
    strcpy(display_line[0], "MAIN MENU ");

    // 3. Move the cursor based on the current selection
    // Note: Everything is strictly padded to 10 characters to clear old pixels!
    if (current_selection == 1) {
        strcpy(display_line[1], ">MISSION  ");
        strcpy(display_line[2], " CALIBRATE");
        strcpy(display_line[3], " IOT MODE ");
        
    } else if (current_selection == 2) {
        strcpy(display_line[1], " MISSION  ");
        strcpy(display_line[2], ">CALIBRATE");
        strcpy(display_line[3], " IOT MODE ");
        
    } else if (current_selection == 3) {
        strcpy(display_line[1], " MISSION  ");
        strcpy(display_line[2], " CALIBRATE");
        strcpy(display_line[3], ">IOT MODE ");
    }
}

void display_menu_mission(int left_val, int right_val, int thumb) {

    if (robot_state == STATE_STOP) {
        static unsigned int frozen_time = 0;
        static int time_is_frozen = 0;
        
        // 1. Freeze the time exactly ONCE when we first enter this state
        if (time_is_frozen == 0) {
            frozen_time = timer_seconds;
            time_is_frozen = 1; // Lock it!
        }

        unsigned int safe_time = frozen_time;
        if (safe_time > 999) safe_time = 999;

        // 2. Draw the exact 10-character custom messages
        strcpy(display_line[0], " BL Stop  ");
        strcpy(display_line[1], " That was ");
        strcpy(display_line[2], "easy!! ;-)");
        strcpy(display_line[3], "Time: 000s");
        display_line[3][6] = (safe_time / 100) + '0';             // Hundreds
        display_line[3][7] = ((safe_time / 10) % 10) + '0';       // Tens
        display_line[3][8] = (safe_time % 10) + '0';              // Ones
        return; 
    }
        
    char left_temp[5];
    char right_temp[5];

    // Format Line 0: Clock
   unsigned int safe_seconds = timer_seconds;
    if (safe_seconds > 999) {
        safe_seconds = 999;
    }


    // Format Line 1: State
    switch(robot_state) {
        case STATE_COUNTDOWN:     strcpy(display_line[0], " BL Start "); break;
        case STATE_DRIVE_TO_LINE: strcpy(display_line[0], " BL Start "); break;
        case STATE_INTERCEPT:     strcpy(display_line[0], " Intercept"); break;
        case STATE_EXECUTE_TURN:  strcpy(display_line[0], " BL Turn  "); break;
        case STATE_LINE_FOLLOW:   
            if (is_circle == 1) {
                strcpy(display_line[0], " BL Circle"); 
            } else {
                strcpy(display_line[0], " BL Travel"); 
            }
            break;
        case STATE_EXIT:         strcpy(display_line[0],  "  BL Exit "); break;
        case STATE_IDLE:          strcpy(display_line[0], "   IDLE   "); break;
        case STATE_STOP:         strcpy(display_line[0],  "  BL Stop "); break;
        default:                  strcpy(display_line[0], " UNKNOWN  "); break;
    }

    // Format Line 2: ADC Hex Values
    HEXtoBCD(left_val);
    strcpy(left_temp, adc_char);
    HEXtoBCD(right_val);
    strcpy(right_temp, adc_char);
   // sprintf(display_line[2], "L%s R%s ", left_temp + 1, right_temp + 1);
        strcpy(display_line[1], " Giovanni ");
        strcpy(display_line[2], "  Yanny   ");


    
    // Format Line 3: Black Calibration Value
   strcpy(display_line[3], "          ");
display_line[3][0] = last_iot_dir_char;

display_line[3][1] = (last_iot_time_units / 10U) + '0';
display_line[3][2] = (last_iot_time_units % 10U) + '0';

display_line[3][3] = ' ';

    display_line[3][4] = (safe_seconds / 100) + '0';            
    display_line[3][5] = ((safe_seconds / 10) % 10) + '0';      
    display_line[3][6] = (safe_seconds % 10) + '0';            

display_line[3][7] = 's';
}

void display_menu_calibrate(int left_val, int right_val, int thumb) {
    char left_temp[5];
    char right_temp[5];

    strcpy(display_line[0], "IR CAL MOD");

    HEXtoBCD(left_val);
    strcpy(left_temp, adc_char);
    HEXtoBCD(right_val);
    strcpy(right_temp, adc_char);

    sprintf(display_line[1], "L%s R%s ", left_temp + 1, right_temp + 1);
    strcpy(display_line[2], "SW1W SW2B ");
    strcpy(display_line[3], "THUMB EXIT");
}

void display_menu_iot(void) {
    
   
    if (iot_course_started == 0) {
        strcpy(display_line[0], " Waiting  ");
        strcpy(display_line[1], "for input ");
        strcpy(display_line[2], " Giovanni ");
        strcpy(display_line[3], "  Yanny   ");
        return; // Exit early!
    }
    
   if (iot_pad_state > 0 && iot_pad_state <= 9) {
        strcpy(display_line[0], "Arrived 0X");
        display_line[0][9] = iot_pad_state + '0';  
    } else {
        strcpy(display_line[0], "          "); 
    }

    // Line 1 & 2: Names
    strcpy(display_line[1], " Giovanni ");
    strcpy(display_line[2], "  Yanny   ");

   
   // Line 3: Last command + continuous timer
unsigned int safe_seconds = (unsigned int)timer_seconds;
if (safe_seconds > 999U) { safe_seconds = 999U; }

strcpy(display_line[3], "          ");   // 10 spaces baseline

// Col 0: direction letter  e.g. 'F'
display_line[3][0] = last_iot_dir_char;

// Col 1-2: time units  e.g. '0','2'
display_line[3][1] = (last_iot_time_units / 10U) + '0';
display_line[3][2] = (last_iot_time_units % 10U) + '0';

// Col 3: space separator
display_line[3][3] = ' ';

// Col 4-6: timer seconds  e.g. '1','2','3'
display_line[3][4] = (safe_seconds / 100U)        + '0';
display_line[3][5] = ((safe_seconds / 10U) % 10U) + '0';
display_line[3][6] = (safe_seconds % 10U)          + '0';

// Col 7: 's'
display_line[3][7] = 's';
}

//------------------------------------------------------------------------------
// Hardware Refresh Trigger
//------------------------------------------------------------------------------
void Display_Process(void) {
    if (update_display) {
        update_display = 0;
        Display_Update(0, 0, 0, 0);
    }
}