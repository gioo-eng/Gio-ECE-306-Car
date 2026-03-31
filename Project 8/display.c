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

extern volatile int timer_minutes;
extern volatile int timer_seconds;

// --- EXTERN VARIABLES (Defined in serial.c) ---
extern volatile unsigned char serial_display_active;
extern char serial_display_line[11];

// --- EXTERN VARIABLES (Defined in system.c) ---
extern volatile unsigned char baud_display_active;
extern char baud_display_line[11];

void Update_Project_Display(int left_val, int right_val, int thumb) {
    char left_temp[5];

    // --- Line 0: Serial input from PC (if active), otherwise Clock ---
    if (serial_display_active) {
        memcpy(display_line[0], serial_display_line, 10);
        display_line[0][10] = '\0';
    } else {
        if (timer_seconds < 10) {
            sprintf(display_line[0], "CLK %d:0%d  ", timer_minutes, timer_seconds);
        } else {
            sprintf(display_line[0], "CLK %d:%d   ", timer_minutes, timer_seconds);
        }
    }

    // --- Line 1: Baud rate (from SW1), otherwise Robot State ---
    if (baud_display_active) {
        memcpy(display_line[1], baud_display_line, 10);
        display_line[1][10] = '\0';
    } else {
        switch(robot_state) {
            case STATE_COUNTDOWN:     sprintf(display_line[1], " COUNT    "); break;
            case STATE_DRIVE_TO_LINE: sprintf(display_line[1], " SEARCH   "); break;
            case STATE_EXECUTE_TURN:  sprintf(display_line[1], "  TURN    "); break;
            case STATE_LINE_FOLLOW:   sprintf(display_line[1], " FOLLOW   "); break;
            case STATE_FORWARD_BURST: sprintf(display_line[1], "  BURST   "); break;
            case STATE_IDLE:          sprintf(display_line[1], "   IDLE   "); break;
            default:                  sprintf(display_line[1], " UNKNOWN  "); break;
        }
    }



    // Flag that the display buffers have been updated
    display_changed = 1;
}

void Display_Process(void) {
    if (update_display) {
        update_display = 0;

        // Prevent SPI bus saturation and screen flicker
        if (display_changed) {
            display_changed = 0; // Clear flag before updating
            Display_Update(0, 0, 0, 0);
        }
    }
}
