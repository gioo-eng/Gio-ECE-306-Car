/*
 * serial.c
 *
 *  Created on: Mar 26, 2026
 *      Author: everettbrostedt
*/

#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

//Char arrays
char speed_1[] = " 115,200  ";
char speed_2[] = " 460,800  ";
char speed_3[] = "  9,600   ";
char repeat[32] = " testing \n";

// Serial Flags
int serial_part = 0;
int receive_flag_0 = 0;
int receive_flag_1 = 0;
unsigned int uca1_flag = 0;
unsigned int serial_state = STARTUP;

unsigned int ok_received = 0;
unsigned int wifi_connected = 0;
unsigned int wifi_address = 0;
unsigned int ip_received = 0;
unsigned int ending_quote = 0;
unsigned int ssid_string_flag = 0;

// Baud config
unsigned int set_baud = 4;
unsigned int tuning = ONE_FIFTEEN;

// Indexes
volatile unsigned int iot_rx_wr = 0;
volatile unsigned int usb_rx_wr = 0;
volatile unsigned int usb_rx_rd = 0;
volatile unsigned int iot_rx_rd = 0;

volatile unsigned int iot_tx = 0;
volatile unsigned int usb_tx = 0;
volatile unsigned int temp_index_usb = 0;
volatile unsigned int temp_index_iot = 0;

int j = 0;
int i = 0;

void Init_Serial_UCA0(int speed){
//------------------------------------------------------------------------------
// TX error (%) RX error (%)
// BRCLK    Baudrate    UCOS16  UCBRx   UCFx    UCSx    neg pos   neg  pos
// 8000000  4800        1       104     2       0xD6 -0.08 0.04 -0.10 0.14
// 8000000  9600        1       52      1       0x49 -0.08 0.04 -0.10 0.14
// 8000000  19200       1       26      0       0xB6 -0.08 0.16 -0.28 0.20
// 8000000  57600       1       8       10      0xF7 -0.32 0.32 -1.00 0.36
// 8000000  115200      1       4       5       0x55 -0.80 0.64 -1.12 1.76
// 8000000  460800      0       17      0       0x4A -2.72 2.56 -3.76 7.28
//------------------------------------------------------------------------------
// Configure eUSCI_A0 for UART mode
    UCA0CTLW0 = 0;
    UCA0CTLW0 |= UCSWRST ;              // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;         // Set SMCLK as fBRCLK
    UCA0CTLW0 &= ~UCMSB;                // MSB, LSB select
    UCA0CTLW0 &= ~UCSPB;                // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    UCA0CTLW0 &= ~UCPEN;                // No Parity
    UCA0CTLW0 &= ~UCSYNC;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 |= UCMODE_0;

    UCA1CTLW0 = 0;
    UCA1CTLW0 |= UCSWRST ;              // Put eUSCI in reset
    UCA1CTLW0 |= UCSSEL__SMCLK;         // Set SMCLK as fBRCLK
    UCA1CTLW0 &= ~UCMSB;                // MSB, LSB select
    UCA1CTLW0 &= ~UCSPB;                // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    UCA1CTLW0 &= ~UCPEN;                // No Parity
    UCA1CTLW0 &= ~UCSYNC;
    UCA1CTLW0 &= ~UC7BIT;
    UCA1CTLW0 |= UCMODE_0;

    UCA0BRW = 4;  //variable that changes the baud rate
    UCA0MCTLW = 0x5551 ; //variable that is change by the switch in order to fine tune the output

    UCA1BRW = 4;  //variable that changes the baud rate
    UCA1MCTLW = 0x5551 ; //variable that is change by the switch in order to fine tune the output

    UCA0CTLW0 &= ~UCSWRST ; // release from reset

    UCA0IE |= UCRXIE;

    UCA1CTLW0 &= ~UCSWRST;
    UCA1IE |= UCRXIE;
    //------------------------------------------------------------------------------
}

#pragma vector = EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void){ //This interrupt is the interrupt relating to serial communication port UCA1
//------------------------------------------------------------------------------
// Interrupt name: eUSCI_A1_ISR
// Description: This interrupt transmits and receives through UCA1
//------------------------------------------------------------------------------
    char usb_value;
    switch(__even_in_range(UCA1IV,0x08)){
        case 0: break; //Vector 0 - no interrupt
        case 2: // Vector 2 – Rx1IFG
            usb_value = UCA1RXBUF;
             USB_Ring_Rx[usb_rx_wr++] = usb_value;
             if(usb_rx_wr >= sizeof(USB_Ring_Rx)){
                 usb_rx_wr = BEGINNING;
             }
             UCA0TXBUF = usb_value;
             break;
         case 4: // Vector 4 – TX1IFG
             UCA1TXBUF = usb_TX_buf[usb_tx];
             usb_TX_buf[usb_tx++] = 0;
             if(usb_TX_buf[usb_tx] == 0x00){
                 UCA1IE &= ~UCTXIE;
                 usb_tx = 0;
             }
         break;
        default:break;
}
//------------------------------------------------------------------------------
}

#pragma vector = EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void){
//This interrupt is the interrupt relating to serial communication port UCA0
//------------------------------------------------------------------------------
// Interrupt name: eUSCI_A0_ISR
// Description: This interrupt transmits and receives through UCA0
//------------------------------------------------------------------------------
    char iot_receive;
    switch (__even_in_range(UCA0IV, 0x08)){
    case 0:
        break; //Vector 0 - no interrupt
    case 2: // Vector 2 – Rx0IFG
        iot_receive = UCA0RXBUF;
        IOT_Ring_Rx[iot_rx_wr++] = iot_receive;
        if (iot_rx_wr >= sizeof(IOT_Ring_Rx)){
            iot_rx_wr = BEGINNING;
        }
        UCA1TXBUF = iot_receive;
        break;
    case 4: // Vector 4 – Tx0IFG
        UCA0TXBUF = iot_TX_buf[iot_tx];

        if (iot_TX_buf[iot_tx] == 0x00){
            iot_tx = 0;
            UCA0IE &= ~UCTXIE;
        }
        iot_TX_buf[iot_tx++] = 0;
        break;
    default:
        break;
    }
}

void serial_update(void){
    if (speed_flag){
        strcpy(speed_type, speed_2);
    }
    else{
        strcpy(speed_type, speed_1);
    }

    switch (serial_state){
        case STARTUP:
            UCA1IE |= UCRXIE;
            if (receive_flag_1){
                receive_flag_1 = 0;
                serial_state = RECEIVE;
                break;
            }
            break;
        case RECEIVE:
            strcpy(iot_TX_buf, display_usb_rx_message);
            serial_state = IOT_STARTUP;
            break;
        case IOT_STARTUP:
            iot_tx = 0;
            UCA0IE |= UCTXIE;
            serial_state = IOT_TRANSMIT;
            break;
        case IOT_TRANSMIT:
            if (receive_flag_0){
                serial_state = USB_TRANSMIT;
                strcpy(usb_TX_buf, display_iot_rx_message);
                UCA1IE |= UCTXIE;
            }
            break;
        case USB_TRANSMIT:
            strcpy(display_line[0], " TRANSMIT ");
            strcpy(display_line[1], display_iot_rx_message);
            strcpy(display_line[2], speed_type);
            strcpy(display_line[3], "          ");
            display_changed = 1;
            serial_state = IDLE;
            break;
        case NEXT_RECEIVE:
            if (receive_flag_1){
                receive_flag_1 = 0;
                serial_state = RECEIVE;
                break;
            }
            break;
        case IDLE:
            receive_flag_0 = 0;
            receive_flag_1 = 0;
            temp_index_usb = 0;
            temp_index_iot = 0;
            serial_state = NEXT_RECEIVE;
            break;
        default:
            break;
        }
}

void rx_process_usb(void){
    unsigned int temp_usb_wr;
    temp_usb_wr = usb_rx_wr;

    if(temp_usb_wr != usb_rx_rd){
        if(USB_Ring_Rx[usb_rx_rd] == '\r' || USB_Ring_Rx[usb_rx_rd] == '\n'){
            receive_flag_1 = 1;
            display_usb_rx_message[temp_index_usb++] = '\0';
            usb_rx_rd++;
            return;
        }
        display_usb_rx_message[temp_index_usb++] = USB_Ring_Rx[usb_rx_rd++];
        if(usb_rx_rd >= sizeof(USB_Ring_Rx)){
               usb_rx_rd = BEGINNING;
           }
    }
}

void rx_process_iot(void){
    unsigned int temp_iot_wr;
    temp_iot_wr = iot_rx_wr;

    if(temp_iot_wr != iot_rx_rd){
        if(IOT_Ring_Rx[iot_rx_rd] == '\r' || IOT_Ring_Rx[iot_rx_rd] == '\n'){
            receive_flag_0 = 1;
            display_iot_rx_message[temp_index_iot++] = '\0';
            iot_rx_rd++;
            return;
        }
        display_iot_rx_message[temp_index_iot++] = IOT_Ring_Rx[iot_rx_rd++];
        if(iot_rx_rd >= sizeof(IOT_Ring_Rx)){
               iot_rx_rd = BEGINNING;
           }
    }
}

void initial_process_iot(void){
    unsigned int temp_iot_wr;
    temp_iot_wr = iot_rx_wr;

    if(temp_iot_wr != iot_rx_rd){
        if(IOT_Ring_Rx[iot_rx_rd] == '\r' || IOT_Ring_Rx[iot_rx_rd] == '\n'){
            if(temp_index_iot){
                char last_char = display_iot_rx_message[temp_index_iot-1];
                receive_flag_0 = 1;

                if(last_char == 'K'){
                    ok_received = 1;
                }
                if(last_char == 'D'){
                    wifi_connected = 1;
                }
                if(last_char == 'P'){
                    ip_received = 1;
                }
                if(ending_quote && wifi_address){
                    display_iot_rx_message[temp_index_iot] = '\0';
                    strcpy(ssid_string, display_iot_rx_message);
                    ssid_string_flag = 1;
                    wifi_address = 0;
                    ending_quote = 0;
                }else{
                    display_iot_rx_message[temp_index_iot] = '\0';
                    temp_index_iot = 0;
                }
            }
            iot_rx_rd++;
            return;
        }

        if((!ssid_string_flag) && (wifi_address || IOT_Ring_Rx[iot_rx_rd] == '\"')){
            if(!wifi_address && IOT_Ring_Rx[iot_rx_rd] == '\"'){
                wifi_address = 1;
                ending_quote = 0;
                temp_index_iot = 0;
                iot_rx_rd++;
                return;
            }
            if(wifi_address){
                if(IOT_Ring_Rx[iot_rx_rd] != '\"' && IOT_Ring_Rx[iot_rx_rd] != ','){
                    display_iot_rx_message[temp_index_iot++] = IOT_Ring_Rx[iot_rx_rd++];
                    if(iot_rx_rd >= sizeof(IOT_Ring_Rx)){
                        iot_rx_rd = BEGINNING;
                    }
                }else{
                    if(ending_quote){
                        display_iot_rx_message[temp_index_iot] = '\0';
                        strcpy(ssid_string,display_iot_rx_message);
                        ssid_string_flag = 1;
                        wifi_address = 0;
                        ending_quote = 0;
                        iot_rx_rd++;
                        return;
                    }
                    ending_quote = 1;
                    iot_rx_rd++;
                    return;
                }
            }

        }else{
            display_iot_rx_message[temp_index_iot++] = IOT_Ring_Rx[iot_rx_rd++];
            if(iot_rx_rd >= sizeof(IOT_Ring_Rx)){
                   iot_rx_rd = BEGINNING;
               }
        }
    }

}


