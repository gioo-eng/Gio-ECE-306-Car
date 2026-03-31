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

volatile uint16_t input_timer = 0;     // counts time
volatile uint8_t input_active = 0;     // flag: user typing

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

    //  115200 baud @ 8MHz (CORRECT)
    UCA1BRW = 4;
    UCA1MCTLW = (0x55 << 8) | UCOS16 | (5 << 4);

    // Pins: P4.2 RX, P4.3 TX
    P4SEL0 |= (BIT2 | BIT3);
    P4SEL1 &= ~(BIT2 | BIT3);

    UCA1CTLW0 &= ~UCSWRST;                // Release reset

    UCA1IFG &= ~(UCRXIFG | UCTXIFG);
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
}

// --- BAUD CHANGE ---
void Serial_setBaud(SerialBaud baud)
{
    serialBaud = baud;
    Init_Serial();
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
        // Echo back to PC
        rb_push(&PCTXBUF, byte);

        // Reset timer on every keypress
        input_timer = 0;
        input_active = 1;

        // Handle ENTER  force immediate display
        if (byte == '\r' || byte == '\n')
        {
            serial_display_active = 1;

            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer,
                   (input_pos < 10) ? input_pos : 10);

            input_pos = 0;
            memset(input_buffer, 0, INPUT_BUFFER_SIZE);

            input_active = 0;
            input_timer = 0;
        }
        else
        {
            // Store character
            if (input_pos < INPUT_BUFFER_SIZE - 1)
            {
                input_buffer[input_pos++] = byte;
            }
        }
    }

    Serial_DrainBufferToUart1(&PCTXBUF);
}

// --- ISR ---
#pragma vector=EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void)
{
    uint8_t rxByte;

    switch (__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
    {
    case USCI_UART_UCRXIFG:

        if (UCA1STATW & (UCOE | UCPE | UCFE))
        {
            (void)UCA1RXBUF; // Clear error
            break;
        }

        rxByte = UCA1RXBUF;

        rb_push(&PCRXBUF, rxByte);

        UCA1IE |= UCTXIE; // Enable TX interrupt
        break;

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

void Serial_TimerTick(void)
{
    if (input_active)
    {
        input_timer++;

        // 5 seconds (adjust based on your timer rate)
        if (input_timer >= 25)
        {
            input_active = 0;
            input_timer = 0;

            serial_display_active = 1;

            memset(serial_display_line, ' ', 10);
            memcpy(serial_display_line, input_buffer,
                   (input_pos < 10) ? input_pos : 10);

            input_pos = 0;
            memset(input_buffer, 0, INPUT_BUFFER_SIZE);
        }
    }
}
