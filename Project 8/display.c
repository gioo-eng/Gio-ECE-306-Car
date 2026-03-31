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
#include "serial.h"
#include <string.h>
#include <stdio.h>

// --- EXTERN VARIABLES (Defined in serial.c) ---
extern volatile unsigned char serial_display_active;
extern char serial_display_line[11];

// --- EXTERN VARIABLES (Defined in system.c) ---
extern volatile unsigned char baud_display_active;
extern char baud_display_line[11];

void Update_Project_Display(int left_val, int right_val, int thumb) {

    // --- Line 0: Serial status (Waiting / Transmit / Received) ---
    switch (serial_ui_state) {
        case SERIAL_STATE_TRANSMIT:
            strcpy(display_line[0], "Transmit  ");
            break;
        case SERIAL_STATE_RECEIVED:
            strcpy(display_line[0], "Received  ");
            break;
        case SERIAL_STATE_WAITING:
        default:
            strcpy(display_line[0], "Waiting   ");
            break;
    }

    // --- Line 1: Transmit preview ---
    if (serial_ui_state == SERIAL_STATE_TRANSMIT) {
        // Show what is being typed live
        memcpy(display_line[1], serial_display_line, 10);
        display_line[1][10] = '\0';
    } else {
        // Clear the line when not transmitting
        strcpy(display_line[1], "          "); 
    }

    // --- Line 2: Baud rate ---
    if (baud_display_active) {
        memcpy(display_line[2], baud_display_line, 10);
        display_line[2][10] = '\0';
    } else {
        if (serialBaud == SERIAL_BAUD_460800)
            strcpy(display_line[2], "460800    ");
        else
            strcpy(display_line[2], "115200    ");
    }

    // --- Line 3: Received data ---
    if (serial_ui_state == SERIAL_STATE_RECEIVED && serial_display_active) {
        memcpy(display_line[3], serial_display_line, 10);
        display_line[3][10] = '\0';
    } else {
        // Clear the line when not receiving
        strcpy(display_line[3], "          ");
    }



    if (serial_ui_state == SERIAL_STATE_TRANSMIT)
{
    // Check if TX buffer drained
    if (PCTXBUF.head == PCTXBUF.tail)
    {
        serial_ui_state = SERIAL_STATE_WAITING;
    }
}



    display_changed = 1;
}

void Display_Process(void) {
    if (update_display) {
        update_display = 0;
        if (display_changed) {
            display_changed = 0;
            Display_Update(0, 0, 0, 0);
        }
    }
}