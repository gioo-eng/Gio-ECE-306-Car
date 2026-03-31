#include "serial.h"
#include "macros.h"
#include <string.h>

RingBuffer PCTXBUF;
RingBuffer PCRXBUF;
SerialMode serialMode = SERIAL_MODE_PC_LOOPBACK;
SerialBaud serialBaud = SERIAL_BAUD_115200;

#define INPUT_BUFFER_SIZE 32
static char input_buffer[INPUT_BUFFER_SIZE];
static uint8_t input_pos = 0;

volatile uint8_t transmit_display_timer = 0;
volatile SerialDisplayState serial_ui_state = SERIAL_STATE_WAITING;
volatile uint16_t input_timer = 0;
volatile uint8_t input_active = 0;

// --- LCD Display ---
volatile unsigned char serial_display_active = 0;
char serial_display_line[11] = "          ";

#define LCD_LINE_WIDTH  (10)
static char serial_lcd_buf[LCD_LINE_WIDTH] = "          ";
static uint8_t serial_lcd_pos = 0;

// --- UART TX Helper ---
static void Serial_DrainBufferToUart1(RingBuffer *buffer)
{
    uint8_t txByte;
    // Check if hardware is ready and if we have data
    while ((UCA1IFG & UCTXIFG) && rb_pop(buffer, &txByte))
    {
        UCA1TXBUF = txByte;
    }
}

// --- UART INIT ---
void InitSerialPCLoopback(void)
{
    UCA1CTLW0 = UCSWRST;                  // Hold in reset
    UCA1CTLW0 |= UCSSEL__SMCLK;           // SMCLK = 8 MHz
    
    // 115200 baud @ 8MHz
    UCA1BRW = 4;
    UCA1MCTLW = (0x55 << 8) | UCOS16 | (5 << 4);
    
    // Pins: P4.2 RX, P4.3 TX
    P4SEL0 |= (BIT2 | BIT3);
    P4SEL1 &= ~(BIT2 | BIT3);

    UCA1CTLW0 &= ~UCSWRST;                // Release reset
    UCA1IFG &= ~(UCRXIFG | UCTXIFG);
    UCA1IFG |= UCTXIFG;                   // Force TX ready
    UCA1IE |= UCRXIE;                     // Enable RX interrupt
}

void Init_Serial(void)
{
    rb_init(&PCTXBUF);
    rb_init(&PCRXBUF);
    memset(serial_lcd_buf, ' ', LCD_LINE_WIDTH);
    serial_lcd_pos = 0;
    InitSerialPCLoopback();
    
    serial_ui_state = SERIAL_STATE_WAITING;
    serial_display_active = 0;
    transmit_display_timer = 0;
}

void Serial_setBaud(SerialBaud baud)
{
    serialBaud = baud;
    Init_Serial();
}

// --- MAIN PROCESS ---
void SerialProcess(void)
{
    uint8_t byte;

    // 1. Handle Incoming Bytes
    while (rb_pop(&PCRXBUF, &byte))
    {
        rb_push(&PCTXBUF, byte);

        // Reset the timer and set state to Transmit so we see the echo
        serial_ui_state = SERIAL_STATE_TRANSMIT;
        transmit_display_timer = 0; 

        __disable_interrupt();
        input_timer = 0;
        input_active = 1;
        __enable_interrupt();

        if (byte == '\r' || byte == '\n')
        {
            input_active = 0;
            input_timer = 0;

            serial_display_active = 0;
            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer, (input_pos < 10) ? input_pos : 10);
            serial_display_line[10] = '\0';
            serial_display_active = 1;

            input_pos = 0;
            memset(input_buffer, 0, INPUT_BUFFER_SIZE);

            // Change to Received once message is complete
            serial_ui_state = SERIAL_STATE_RECEIVED;
        }
        else if (byte == 0x7F || byte == '\b')
        {
            if (input_pos > 0) input_pos--;
        }
        else
        {
            if (input_pos < INPUT_BUFFER_SIZE - 1)
                input_buffer[input_pos++] = byte;
        }
    }

    // 2. Transmit bytes to PC
    Serial_DrainBufferToUart1(&PCTXBUF);

    // 3. UI State Management (THE DELAY LOGIC)
    // If we are in Transmit mode, stay there until the timer hits 10 ticks
    if (serial_ui_state == SERIAL_STATE_TRANSMIT)
    {
        if (transmit_display_timer >= 10)  // 10 ticks (e.g., 2 seconds)
        {
            transmit_display_timer = 0;
            serial_ui_state = SERIAL_STATE_WAITING;
        }
    }
}

// --- ISR ---
#pragma vector=EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void)
{
    uint8_t rxByte;
    uint8_t txByte;
    switch (__even_in_range(UCA1IV, 0x08))
    {
    case USCI_UART_UCRXIFG:
        rxByte = UCA1RXBUF;
        rb_push(&PCRXBUF, rxByte);
        break;

    case USCI_UART_UCTXIFG:
        if (rb_pop(&PCTXBUF, &txByte))
        {
            UCA1TXBUF = txByte;
        }
        else
        {
            UCA1IE &= ~UCTXIE;
            UCA1IFG |= UCTXIFG; // Keep flag set so next enable triggers instantly
        }
        break;
    }
}

// --- TIMER TICK (Call from Timer ISR) ---
void Serial_TimerTick(void)
{
    // Increment timer ONLY if we are in transmit state
    if (serial_ui_state == SERIAL_STATE_TRANSMIT)
    {
        transmit_display_timer++;
    }

    if (input_active)
    {
        input_timer++;
        if (input_timer >= 25) 
        {
            input_active = 0;
            input_timer = 0;
            serial_ui_state = SERIAL_STATE_WAITING;
        }
    }
}

// --- SW2 TRANSMIT ---
void Serial_TransmitBuffer(void)
{
    uint8_t i;
    uint8_t len = 10;

    // Allow transmission from Received state
    if (serial_ui_state != SERIAL_STATE_RECEIVED)
        return;

    // Find end of text (skip trailing spaces)
    while (len > 0 && serial_display_line[len - 1] == ' ')
        len--;

    if (len == 0) return;

    for (i = 0; i < len; i++)
        rb_push(&PCTXBUF, (uint8_t)serial_display_line[i]);

    rb_push(&PCTXBUF, '\r');
    rb_push(&PCTXBUF, '\n');

    // Trigger the "Wait" period for the LCD
    transmit_display_timer = 0; 
    serial_ui_state = SERIAL_STATE_TRANSMIT;
    
    // Enable interrupt to start sending
    UCA1IE |= UCTXIE;
}