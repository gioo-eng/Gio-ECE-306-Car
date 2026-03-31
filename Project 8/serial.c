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
    while ((UCA1IFG & UCTXIFG) && rb_pop(buffer, &txByte))
    {
        UCA1TXBUF = txByte;
    }
}

// --- UART INIT (PC - UCA1) ---
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

// --- INIT WRAPPER ---
void Init_Serial(void)
{
    rb_init(&PCTXBUF);
    rb_init(&PCRXBUF);
    memset(serial_lcd_buf, ' ', LCD_LINE_WIDTH);
    serial_lcd_pos = 0;
    InitSerialPCLoopback();
    UCA1IFG |= UCTXIFG;
    serial_ui_state = SERIAL_STATE_WAITING;
    serial_display_active = 0;
}

// --- BAUD CHANGE ---
void Serial_setBaud(SerialBaud baud)
{
    serialBaud = baud;
    Init_Serial();
    UCA1IFG |= UCTXIFG;    // force TX ready after baud change
}

void Serial_setMode(SerialMode mode)
{
    serialMode = mode;
}

// --- MAIN PROCESS ---
void SerialProcess(void)
{
    uint8_t byte;

    while (rb_pop(&PCRXBUF, &byte))
    {
        rb_push(&PCTXBUF, byte);

        serial_ui_state = SERIAL_STATE_TRANSMIT;

        __disable_interrupt();
        input_timer = 0;
        input_active = 1;
        __enable_interrupt();

        if (byte == '\r' || byte == '\n')
        {
            __disable_interrupt();
            input_active = 0;
            input_timer = 0;
            __enable_interrupt();

            serial_display_active = 0;
            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer,
                   (input_pos < 10) ? input_pos : 10);
            serial_display_line[10] = '\0';
            serial_display_active = 1;

            input_pos = 0;
            memset(input_buffer, 0, INPUT_BUFFER_SIZE);

            serial_ui_state = SERIAL_STATE_RECEIVED;
        }
        else if (byte == 0x7F || byte == '\b')
        {
            if (input_pos > 0)
                input_pos--;

            serial_display_active = 0;
            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer,
                   (input_pos < 10) ? input_pos : 10);
            serial_display_line[10] = '\0';
            serial_display_active = 1;
        }
        else
        {
            if (input_pos < INPUT_BUFFER_SIZE - 1)
                input_buffer[input_pos++] = byte;

            serial_display_active = 0;
            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer,
                   (input_pos < 10) ? input_pos : 10);
            serial_display_line[10] = '\0';
            serial_display_active = 1;
        }
    }

    Serial_DrainBufferToUart1(&PCTXBUF);

    // Return to waiting once TX buffer fully drained
    if (serial_ui_state == SERIAL_STATE_TRANSMIT)
    {
        if (PCTXBUF.head == PCTXBUF.tail)
        {
            serial_ui_state = SERIAL_STATE_WAITING;
            memset(serial_display_line, ' ', 10);
            serial_display_line[10] = '\0';
            serial_display_active = 0;
        }
    }
}

// --- ISR ---
#pragma vector=EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void)
{
    uint8_t rxByte;
    switch (__even_in_range(UCA1IV, 0x08))
    {
    case USCI_UART_UCRXIFG:
        rxByte = UCA1RXBUF;        // Always read first to clear flags
        rb_push(&PCRXBUF, rxByte); // Always push — no error check dropping bytes
        break;                     // ← removed UCTXIE enable from RX case

    case USCI_UART_UCTXIFG:
        if (rb_pop(&PCTXBUF, &rxByte))
        {
            UCA1TXBUF = rxByte;
        }
        else
        {
            UCA1IE &= ~UCTXIE;
        }
        break;

    default:
        break;
    }
}

// --- TIMER TICK ---
void Serial_TimerTick(void)
{
    if (input_active)
    {
        input_timer++;
        if (input_timer >= 25)
        {
            input_active = 0;
            input_timer = 0;

            serial_display_active = 0;
            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer,
                   (input_pos < 10) ? input_pos : 10);
            serial_display_line[10] = '\0';
            serial_display_active = 1;

            input_pos = 0;
            memset(input_buffer, 0, INPUT_BUFFER_SIZE);

            serial_ui_state = SERIAL_STATE_WAITING;
        }
    }
}

// --- SW2 TRANSMIT ---
void Serial_TransmitBuffer(void)
{
    uint8_t i;
    char temp[11];
    uint8_t len;

    if (serial_ui_state != SERIAL_STATE_RECEIVED)
        return;

    memcpy(temp, serial_display_line, 10);
    temp[10] = '\0';

    len = 10;
    while (len > 0 && temp[len - 1] == ' ')
        len--;

    if (len == 0)
        return;

    for (i = 0; i < len; i++)
        rb_push(&PCTXBUF, (uint8_t)temp[i]);

    rb_push(&PCTXBUF, '\r');
    rb_push(&PCTXBUF, '\n');

    UCA1IE |= UCTXIE;
    serial_ui_state = SERIAL_STATE_TRANSMIT;
}