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

extern volatile int timer_minutes;
extern volatile int timer_seconds;

// --- New Global Variables for Menu System ---
volatile int menu_mode = 0;         // 0 = Navigating, 1 = Mission, 2 = Calibrating, 3 = IOT
volatile int current_selection = 1; // 1 = Mission, 2 = IR Cal, 3 = IOT
volatile int WHITE = 0;             // Saved white calibration value
volatile int BLACK = 0;             // Saved black calibration value
char last_cmd_display[11] = "WAITING   ";

// Variables to let the ISR access current sensor states
volatile int last_thumb = 0;
volatile int cal_entry_thumb = 0;
volatile int last_left = 0;
volatile int last_right = 0;


void Update_Project_Display(int left_val, int right_val, int thumb) {
    char left_temp[5];
    char right_temp[5];
    int i; // Used for our manual string copying
    const char *last_cmd; // Used to hold the UART frame pointer

    last_thumb = thumb;
    last_left = left_val;
    last_right = right_val;

    // --- EXIT LOGIC FOR MODES 2 AND 3 ---
    if (menu_mode == 2 || menu_mode == 3) {
        if (thumb > (cal_entry_thumb + 3) || thumb < (cal_entry_thumb - 3)) {
            menu_mode = 0; 
        }
    }

    // --- NAVIGATION THUMBWHEEL LOGIC ---
    if (menu_mode == 0) {
        if (thumb < 85) {
            current_selection = 1; // MISSION
        } else if (thumb < 170) {
            current_selection = 2; // IR CAL
        } else {
            current_selection = 3; // IOT MODE
        }
    }

    if (menu_mode == 0) {
        // --- NAVIGATION MENU DISPLAY ---
        strcpy(display_line[0], "MENU      ");

        if (current_selection == 1) {
            strcpy(display_line[1], "MISSION   ");
            strcpy(display_line[2], "RUN ROUTE ");
        } else if (current_selection == 2) {
            strcpy(display_line[1], "IR CAL    ");
            strcpy(display_line[2], "WHT+BLK   ");
        } else {
            strcpy(display_line[1], "IOT MODE  ");
            strcpy(display_line[2], "WIFI INFO ");
        }
        strcpy(display_line[3], "SW1 SELECT");

    } else if (menu_mode == 1) {
        // --- MISSION MODE DISPLAY ---
        if (timer_seconds < 10) {
            sprintf(display_line[0], "CLK %d:0%d  ", timer_minutes, timer_seconds);
        } else {
            sprintf(display_line[0], "CLK %d:%d   ", timer_minutes, timer_seconds);
        }

        switch(robot_state) {
            case STATE_COUNTDOWN:     strcpy(display_line[1], " COUNT    "); break;
            case STATE_DRIVE_TO_LINE: strcpy(display_line[1], " SEARCH   "); break;
            case STATE_EXECUTE_TURN:  strcpy(display_line[1], "  TURN    "); break;
            case STATE_LINE_FOLLOW:   strcpy(display_line[1], " FOLLOW   "); break;
            case STATE_FORWARD_BURST: strcpy(display_line[1], "  BURST   "); break;
            case STATE_IDLE:          strcpy(display_line[1], "   IDLE   "); break;
            default:                  strcpy(display_line[1], " UNKNOWN  "); break;
        }

        HEXtoBCD(left_val);
        strcpy(left_temp, adc_char);

        HEXtoBCD(right_val);
        strcpy(right_temp, adc_char);

        sprintf(display_line[2], "L%s R%s ", left_temp + 1, right_temp + 1);

        HEXtoBCD(thumb);
        sprintf(display_line[3], "TW: %s   ", adc_char + 1);

    } else if (menu_mode == 2) {
        // --- CALIBRATION MODE DISPLAY ---
        strcpy(display_line[0], "IR CAL MOD");

        HEXtoBCD(left_val);
        strcpy(left_temp, adc_char);

        HEXtoBCD(right_val);
        strcpy(right_temp, adc_char);

        sprintf(display_line[1], "L%s R%s ", left_temp + 1, right_temp + 1);
        strcpy(display_line[2], "SW1W SW2B ");
        strcpy(display_line[3], "THUMB EXIT");
        
    } else if (menu_mode == 3) {
        // --- IOT MODULE STATUS DISPLAY (NO MODIFIERS) ---
        
        // 1. Wipe all 4 lines with exactly 10 spaces
        strcpy(display_line[0], "          ");
        strcpy(display_line[1], "          ");
        strcpy(display_line[2], "          ");
        strcpy(display_line[3], "          ");

        // 2. Line 0: Overwrite spaces with the SSID
        for (i = 0; i < 10 && ESP_WIFI_SSID[i] != '\0'; i++) {
            display_line[0][i] = ESP_WIFI_SSID[i];
        }

        // 3. Line 1: Overwrite spaces with the IP Address
        for (i = 0; i < 10 && ESP_IP_ADDRESS[i] != '\0'; i++) {
            display_line[1][i] = ESP_IP_ADDRESS[i];
        }

        // 4. Line 2: Overwrite spaces with the Raw UART Frame (e.g., "0,CONNECT")
        last_cmd = uart_get_last_frame();
        for (i = 0; i < 10 && last_cmd[i] != '\0'; i++) {
            display_line[2][i] = last_cmd[i];
        }

        // 5. Line 3: Overwrite spaces with the Parsed Command (e.g., "F4")
        for (i = 0; i < 10 && last_cmd_display[i] != '\0'; i++) {
            display_line[3][i] = last_cmd_display[i];
        }
    }

    display_changed = 1;
}

void Display_Process(void) {
    if (update_display) {
        update_display = 0;
        Display_Update(0, 0, 0, 0);
    }
}