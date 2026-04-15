/*
 * globals.h
 *
 *  Created on: Mar 1, 2026
 *      Author: everettbrostedt
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

extern volatile unsigned int Time_Sequence;
extern volatile char one_time;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern char display_line[4][11];
extern char *display[4];
extern unsigned char display_mode_i;
extern unsigned int project_5_flag;
extern volatile unsigned int one_second;
extern volatile unsigned int zero_point_one;

extern unsigned char event;
extern unsigned int state;
extern volatile int mode;
extern unsigned int movement;
extern unsigned int start_flag;

extern unsigned int cycle_time;
extern unsigned int time_change;

extern volatile unsigned int ADC_Thumb;
extern volatile unsigned int ADC_Left_Detect;
extern volatile unsigned int ADC_Right_Detect;

extern volatile unsigned char left_flag;
extern volatile unsigned char right_flag;
extern volatile unsigned char thumb_flag;

extern char thousands;
extern char hundreds;
extern char tens;
extern char ones;

extern volatile int condition;
extern int delay_amount;
extern int small_delay_amount;
extern unsigned int line_detection_flag;
extern unsigned int first;

//Speed from PWM (for ramp up)
extern volatile int left_forward;
extern volatile int right_forward;
extern volatile int left_reverse;
extern volatile int right_reverse;

extern volatile unsigned char speed_update;

//DAC variables
extern unsigned int DAC_data;

extern unsigned int black_value_R;
extern unsigned int black_value_L;
extern unsigned int white_value_R;
extern unsigned int white_value_L;

//Serial Communication Globals
extern char repeat[32];
extern char speed_1[];
extern char speed_2[];
extern char speed_3[];

extern unsigned int set_baud;
extern unsigned int tuning;
extern int speed_flag;

//Serial Flags
extern unsigned int ok_received;
extern unsigned int wifi_connected;
extern unsigned int wifi_address;
extern unsigned int ip_received;
extern unsigned int ending_quote;
extern unsigned int ssid_string_flag;

extern unsigned int serial_state;
extern unsigned int uca1_flag;
extern int receive_flag_0;
extern char display_rx_message[32];
extern char display_tx_message[32];

extern volatile unsigned int iot_rx_wr;
extern volatile unsigned int usb_rx_wr;
extern volatile unsigned int iot_tx;
extern volatile unsigned int direct_iot;
extern volatile unsigned int temp_index_usb;
extern volatile unsigned int temp_index_iot;

//MSP430 stores
char display_usb_rx_message[32];
char display_iot_rx_message[32];
char ssid_string[32];
char ip_string[32];
char speed_type[11];
char safe[32];

//Buffers
char IOT_Ring_Rx[32];
char USB_Ring_Rx[32];
char iot_TX_buf[32];
char usb_TX_buf[32];

//Switch global variables
extern unsigned int sw1_position;
extern unsigned int number_of_presses;
extern unsigned int number_of_presses_2;

#endif /* GLOBALS_H_ */





