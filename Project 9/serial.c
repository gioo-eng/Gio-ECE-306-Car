#include "serial.h"
#include "ports.h"
#include "macros.h"
#include <msp430.h>

#define RING_MASK (RING_SZ - 1)

// --- UCA0 (IoT / ESP32) Variables ---
static volatile char ring[RING_SZ];
static volatile uint8_t ring_head = 0;
static volatile uint8_t ring_tail = 0;
static volatile uint8_t frames_queued = 0;
static char last_frame[BUF_LEN];

static void (*s_iot_rx_hook)(char) = 0;

void uart_set_iot_rx_hook(void (*hook)(char))
{
    s_iot_rx_hook = hook;
}

static uint8_t ring_available(void) {
    return (ring_head - ring_tail) & RING_MASK;
}

void uart_init(void) {
    // --- Initialize UCA0 (ESP32 Module) ---
    UCA0CTL1 |=  UCSWRST;
    UCA0CTL1  =  UCSSEL_2 | UCSWRST;
    UCA0CTL0  =  0;
    UCA0BRW   =  4;
    UCA0MCTLW =  (5 << 8) | (3 << 4) | UCOS16; // 115200 default
    P1SEL0   |=  UCA0RXD | UCA0TXD;
    P1SEL1   &= ~(UCA0RXD | UCA0TXD);
    UCA0CTL1 &= ~UCSWRST;
    UCA0IE   |=  UCRXIE;

    // --- Initialize UCA1 (PC / Serial Monitor) ---
    UCA1CTL1 |=  UCSWRST;
    UCA1CTL1  =  UCSSEL_2 | UCSWRST;
    UCA1CTL0  =  0;
    UCA1BRW   =  4;
    UCA1MCTLW =  (5 << 8) | (3 << 4) | UCOS16; // 115200 default
    P1SEL0   |=  UCA1RXD | UCA1TXD; 
    P1SEL1   &= ~(UCA1RXD | UCA1TXD);
    UCA1CTL1 &= ~UCSWRST;
    UCA1IE   |=  UCRXIE;
}


void uart_send_buf(const char *src) {
    while (*src != '\0') {
        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = *src++;
    }
    while (!(UCA0IFG & UCTXIFG)); UCA0TXBUF = 0x0D; // \r
    while (!(UCA0IFG & UCTXIFG)); UCA0TXBUF = 0x0A; // \n
}

uint8_t uart_read_frame(char *dst) {
    uint8_t i = 0;
    char temp;
    uint8_t found_delim = 0;

    if (frames_queued == 0) { return 0; }

    do {
        i = 0;
        found_delim = 0;

        while (ring_available() > 0) {
            temp = ring[ring_tail++ & RING_MASK];
            if (temp == '\n') { found_delim = 1; break; }
            if (i < (BUF_LEN - 1) && temp != '\r') {
                dst[i]        = temp;
                last_frame[i] = temp;
                i++;
            }
        }

        dst[i]        = '\0';
        last_frame[i] = '\0';

        if (found_delim && frames_queued > 0) { frames_queued--; }

    } while (i == 0 && found_delim && frames_queued > 0);

    return (i > 0) ? 1U : 0U;
}

const char *uart_get_last_frame(void) { return last_frame; }

// --- UCA0 ISR (ESP32 Module -> PC & Robot) ---
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCI_A0_RX_ISR(void) {
    if (UCA0IFG & UCRXIFG) {
        char rx = UCA0RXBUF;
        
        // 1. Save byte for the robot's main loop to parse
        ring[ring_head++ & RING_MASK] = rx;
        if (rx == '\n') frames_queued++;
        if (s_iot_rx_hook) { s_iot_rx_hook(rx); }
        
        // 2. Forward the byte to the PC so you can read it!
        while (!(UCA1IFG & UCTXIFG)); // Wait for UCA1 TX to be ready
        UCA1TXBUF = rx;               // Send to PC
    }
}

// --- UCA1 ISR (PC -> ESP32 Module) ---
#pragma vector=EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void)
{
    switch (__even_in_range(UCA1IV, 0x08))
    {
        case USCI_UART_UCRXIFG: // We received a keystroke from the PC
            
            // Forward the keystroke directly to the ESP32
            while (!(UCA0IFG & UCTXIFG)); // Wait for UCA0 TX to be ready
            UCA0TXBUF = UCA1RXBUF;        // Send to ESP32
            break;

        case USCI_UART_UCTXIFG:
            break;
    }
}