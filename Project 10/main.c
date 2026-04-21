//------------------------------------------------------------------------------
//  Jim Carlson
//  Jan 2023
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

#include "msp430.h"
#include <string.h>
#include "functions.h"
#include "LCD.h"
#include "ports.h"
#include "macros.h"
#include "timers.h"
#include "motors.h"
#include "esp.h"
#include "serial.h" 

// --- Variables needed for Main Loop Timing ---
volatile unsigned int Time_Sequence  = 0;
unsigned int Last_Time_Sequence = 0;
unsigned int cycle_time = 0;
unsigned int time_change = 0;

// --- Variables needed for LCD Display ---
extern volatile int left_ir;
extern volatile int right_ir;
extern volatile int thumb;

// --- Variables needed for State Machine Control ---
extern volatile int system_running;
extern volatile unsigned int robot_state;

// --- Variables for IoT Serial Processing ---
char uart_rx_buffer[BUF_LEN];
ESPCommandEvent current_event;
void Init_DAC(void);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; 
    PM5CTL0 &= ~LOCKLPM5;

    Init_Ports();
    Init_Clocks();
    Init_Conditions();
    Init_Timers();
    Init_LCD();
    init_adc();
    Init_DAC();
    uart_init();
    __enable_interrupt();
    ESP_Init();
    turn_off_all();
    ms_delay(800);
    P3OUT |= IOT_EN;

    Time_Sequence = 0;
    strcpy(display_line[0], "          ");
    strcpy(display_line[1], "          ");
    strcpy(display_line[2], "          ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;

    check_motor_safety();

    //--------------------------------------------------------------------------
    // Main Loop
    //--------------------------------------------------------------------------
    while (ALWAYS) {
        check_motor_safety();

       // --- NEW: ESP SERIAL PROCESSING ---
        // 1. Drain the ring buffer and process any complete frames
        if (uart_read_frame(uart_rx_buffer)) {
            ESP_ProcessStartup(uart_rx_buffer);
            ESP_EnqueueFromFrame(uart_rx_buffer);
        }

        // 2. Process Commands IMMEDIATELY (Removed the lockout!)
        if (ESP_DequeueCommand(&current_event)) {
            if (current_event.valid) {
                execute_iot_command(&current_event);
                current_event.valid = 0; 
            }
        }

        // Update timing flags
        if (Last_Time_Sequence != Time_Sequence) {
            Last_Time_Sequence = Time_Sequence;
            cycle_time++;
            time_change = 1;
        } 

       if (system_running) {
            Run_Mission_Machine(); 
        } else {
            turn_off_all();
            robot_state = STATE_IDLE;
        }

        Update_Project_Display(left_ir, right_ir, thumb);
        Display_Process();
        P3OUT ^= TEST_PROBE;
    }
}