/*
 * iot.c
 *
 *  Created on: Mar 27, 2026
 *      Author: everettbrostedt
 */

#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

int input_rd = 0;
unsigned int command_flag = 0;
unsigned int iot_state = 0;
unsigned int iot_status = IDLE;
unsigned int iot_startup_timer = 0;
unsigned int configured_flag = 0;
char iot_input;
int command_1 = 0;
int command_2 = 0;
unsigned int run_time = 0;
unsigned int run_time3 = 0;
unsigned int run_time2 = 0;
unsigned int run_time1 = 0;
unsigned int run_time0 = 0;

unsigned int command_counter = 0;

unsigned int advance_0 = 0;
unsigned int advance_1 = 0;
unsigned int advance_2 = 0;
unsigned int advance_3 = 0;

static void reset_unlock_sequence(void){
    advance_0 = 0;
    advance_1 = 0;
    advance_2 = 0;
    advance_3 = 0;
}

static void reset_command_values(void){
    command_flag = 0;
    iot_state = IDLE;
    command_1 = 0;
    command_2 = 0;
    run_time = 0;
    run_time3 = 0;
    run_time2 = 0;
    run_time1 = 0;
    run_time0 = 0;
    command_counter = 0;
}

static void update_run_time(char iot_value){
    run_time3 = run_time2;
    run_time2 = run_time1;
    run_time1 = run_time0;
    run_time0 = (unsigned int)(iot_value - '0');
    run_time = (run_time3 * 1000) + (run_time2 * 100) + (run_time1 * 10) + run_time0;
}

void IOT_State(void){
    switch(iot_status){
        case IDLE:
            P3OUT &= ~IOT_EN;
            zero_point_one = 0;
            iot_status = IOT_STARTUP;
            iot_startup_timer = 0;
            break;
        case IOT_STARTUP:
            if(zero_point_one){
                zero_point_one = 0;
                iot_startup_timer++;
                if(iot_startup_timer >= 2){
                    P3OUT |= IOT_EN;
                }
                if(iot_startup_timer >= 4){
                    temp_index_iot = 0;
                    temp_index_usb = 0;
                    configured_flag = 0;
                    iot_startup_timer = 0;
                    iot_status = COMMAND_START;
                }
            }
            break;
        case COMMAND_START:
            initial_process_iot();
            if(wifi_connected && ip_received){
                wifi_connected = 0;
                ip_received = 0;
                strcpy(iot_TX_buf, "AT+SYSSTORE=0\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = MUX_RUN;
            }
            break;
        case MUX_RUN:
            initial_process_iot();
            if(ok_received){
                ok_received = 0;
                strcpy(iot_TX_buf, "AT+CIPMUX=1\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = SERVER_RUN;
            }
            break;
        case SERVER_RUN:
            initial_process_iot();
            if(ok_received){
                ok_received = 0;
                strcpy(iot_TX_buf, "AT+CIPSERVER=1,1932\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = IOT_READY;
            }
            break;
        case IOT_READY:
            initial_process_iot();
            if(ok_received){
                strcpy(display_line[2], "IOT READY!");
                display_changed = 1;

                configured_flag = 1;
                iot_status = IOT_DISPLAY;
            }
            break;
        case IOT_DISPLAY:
            if(configured_flag && sw1_position){
                configured_flag = 0;
                sw1_position = 0;
                ok_received = 0;

                strcpy(iot_TX_buf, "AT+CWJAP?\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = SSID;
            }
            break;
        case SSID:
            initial_process_iot();
            if(ssid_string_flag){
                display_line[0][0] = 'S';
                display_line[0][1] = 'S';
                display_line[0][2] = 'I';
                display_line[0][3] = 'D';
                display_line[0][4] = ':';
                display_line[0][5] = ssid_string[0];
                display_line[0][6] = ssid_string[1];
                display_line[0][7] = ssid_string[2];
                display_line[0][8] = ssid_string[3];
                display_line[0][9] = '\0';
                strcpy(display_line[2], "          ");
                display_changed = 1;
                if(ok_received){
                    ok_received = 0;
                    wifi_address = 0;
                    ssid_string_flag = 0;
                    ending_quote = 0;
                    temp_index_iot = 0;

                    strcpy(iot_TX_buf, "AT+CIFSR\r\n");
                    iot_tx = 0;
                    UCA0IE |= UCTXIE;
                    iot_status = IP_ADDY;
                }
            }
            break;
        case IP_ADDY:
            initial_process_iot();
            if(ssid_string_flag){
                strcpy(display_line[1], "IP: ");
                display_line[2][0] = ssid_string[0];
                display_line[2][1] = ssid_string[1];
                display_line[2][2] = ssid_string[2];
                display_line[2][3] = ssid_string[3];
                display_line[2][4] = ssid_string[4];
                display_line[2][5] = ssid_string[5];
                display_line[2][6] = ssid_string[6];
                display_line[2][7] = ssid_string[7];
                display_line[2][8] = ssid_string[8];
                display_line[2][9] = ssid_string[9];
                display_line[3][0] = ssid_string[10];
                display_line[3][1] = ssid_string[11];
                display_line[3][2] = ssid_string[12];
                display_line[3][3] = ssid_string[13];
                display_changed = 1;
            }
            if(ok_received){
                ok_received = 0;
                ssid_string_flag = 0;
                wifi_address = 0;
                ending_quote = 0;
                temp_index_iot = 0;
                input_rd = 0;
                temp_index_iot = 0;
                command_flag = 0;
                iot_status = IOT_RUN;
            }
            break;
        case IOT_RUN:
            rx_process_iot();
            IOT_Process();
            break;
        default:break;
    }
}

void IOT_Process(void){
    if (command_flag && (iot_state != RECEIVE)){
        switch (iot_state){
            case '^':
                uca1_flag = 1;
                strcpy(repeat, " I'm here bbg \n\0");
                no_movement();
                reset_command_values();
                break;
            case 'F':
                strcpy(display_line[0], "Forward   ");
                display_changed = 1;
                medium_forward();
                if(one_second){
                    one_second = 0;
                    command_counter++;
                }

                if(command_counter >= run_time){
                    no_movement();
                    command_counter = 0;
                    if(command_2){
                        iot_state = command_2;
                        command_2 = 0;
                    }else{
                        reset_command_values();
                    }
                }
                break;
            case 'B':
                strcpy(display_line[0], "Backwards ");
                display_changed = 1;
                medium_reverse();
                if(one_second){
                    one_second = 0;
                    command_counter++;
                }

                if(command_counter >= run_time){
                    no_movement();
                    command_counter = 0;
                    if(command_2){
                        iot_state = command_2;
                        command_2 = 0;
                    }else{
                        reset_command_values();
                    }
                }
                break;
            case 'L':
                strcpy(display_line[0], "Left      ");
                display_changed = 1;
                medium_left();
                if(zero_point_one){
                    zero_point_one = 0;
                    command_counter++;
                }

                if(command_counter >= run_time){
                    no_movement();
                    command_counter = 0;
                    if(command_2){
                        iot_state = command_2;
                        command_2 = 0;
                    }else{
                        reset_command_values();
                    }
                }
                break;
            case 'R':
                strcpy(display_line[0], "Right     ");
                display_changed = 1;
                medium_right();
                if(one_second){
                    one_second = 0;
                    command_counter++;
                }

                if(command_counter >= run_time){
                    no_movement();
                    command_counter = 0;
                    if(command_2){
                        iot_state = command_2;
                        command_2 = 0;
                    }else{
                        reset_command_values();
                    }
                }
                break;
            default:
                no_movement();
                reset_command_values();
                break;
        }
        return;
    }

    if(!receive_flag_0){
        return;
    }

    input_rd = BEGINNING;

    while((iot_input = display_iot_rx_message[input_rd++]) != '\0'){
        if(command_flag && (iot_state == RECEIVE)){
            if((iot_input == 'F') || (iot_input == 'B') || (iot_input == 'L') || (iot_input == 'R') || (iot_input == '^')){
                if(!command_1){
                    command_1 = iot_input;
                }
                else if(!command_2){
                    command_2 = iot_input;
                }
            }
            else if((iot_input >= '0') && (iot_input <= '9')){
                update_run_time(iot_input);
            }
            continue;
        }

        if (iot_input == '^'){
            reset_unlock_sequence();
            advance_0 = 1;
            continue;
        }
        if (iot_input == '0' && advance_0){
            advance_1 = 1;
            continue;
        }
        if (iot_input == '8' && advance_1){
            advance_2 = 1;
            continue;
        }
        if (iot_input == '6' && advance_2){
            advance_3 = 1;
            continue;
        }
        if (iot_input == '4' && advance_3){
            command_flag = 1;
            iot_state = RECEIVE;
            command_counter = 0;
            command_1 = 0;
            command_2 = 0;
            run_time = 0;
            run_time3 = 0;
            run_time2 = 0;
            run_time1 = 0;
            run_time0 = 0;
            continue;
        }
        reset_unlock_sequence();
    }

    receive_flag_0 = 0;
    temp_index_iot = 0;
    input_rd = BEGINNING;
    display_iot_rx_message[BEGINNING] = '\0';
    reset_unlock_sequence();

    if(command_flag && (iot_state == RECEIVE) && command_1){
        if((command_1 != '^') && (run_time == 0)){
            run_time = 1;
        }
        iot_state = command_1;
    }else if(command_flag && (iot_state == RECEIVE)){
        reset_command_values();
    }
}
