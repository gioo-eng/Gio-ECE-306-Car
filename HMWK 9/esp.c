// ---------------------------------------------------------------------------
//  esp.c
//
//  Owns all serial communication for the project.
//  Replaces ringbuffer.c and serial.c entirely.
//
//  UCA1 = PC  port (USB)
//  UCA0 = IOT port (ESP32)
//
//  ^ Command set (PC → FRAM only, never forwarded to ESP32):
//    ^^  →  "I'm here"
//    ^F  →  set UCA0 to 115,200  →  "115,200"
//    ^S  →  set UCA0 to 9,600    →  "9,600"
//    ^1234F5678 → Robot Control (Param1, Command, Param2)
// ---------------------------------------------------------------------------

#include "esp.h"
#include "msp430.h"
#include <string.h>

// ===========================================================================
//  Ring Buffer Implementation  (formerly ringbuffer.c)
// ===========================================================================

void rb_init(RingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    memset((void *)rb->buffer, 0, BUFFER_SIZE);
}

uint8_t rb_push(RingBuffer *rb, uint8_t data)
{
    uint8_t next = (rb->head + 1) & BUFFER_MASK;
    if (next == rb->tail) return 0;     // Full — drop byte
    rb->buffer[rb->head] = data;
    rb->head = next;
    return 1;
}

uint8_t rb_pop(RingBuffer *rb, uint8_t *dest)
{
    if (rb->head == rb->tail) return 0; // Empty
    *dest = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & BUFFER_MASK;
    return 1;
}

// ===========================================================================
//  Ring Buffers
// ===========================================================================
RingBuffer PCTXBUF;     // FRAM → PC     (UCA1 TX)
RingBuffer PCRXBUF;     // PC   → FRAM   (UCA1 RX)
RingBuffer IOTTXBUF;    // FRAM → ESP32  (UCA0 TX)
RingBuffer IOTRXBUF;    // ESP32 → FRAM  (UCA0 RX)

// ===========================================================================
//  Serial (UCA1) State  (formerly serial.c globals)
// ===========================================================================
#define INPUT_BUFFER_SIZE 32
static char    input_buffer[INPUT_BUFFER_SIZE];
static uint8_t input_pos = 0;

volatile uint8_t           transmit_display_timer = 0;
volatile SerialDisplayState serial_ui_state        = SERIAL_STATE_WAITING;
volatile uint16_t          input_timer             = 0;
volatile uint8_t           input_active            = 0;

volatile unsigned char serial_display_active       = 0;
char                   serial_display_line[11]     = "          ";

// ===========================================================================
//  ESP (UCA0) Command Parser State
// ===========================================================================
#define CMD_BUF_SIZE 32

typedef enum {
    ESP_CMD_IDLE,
    ESP_CMD_WAIT,
    ESP_CMD_BUILD
} EspCmdState;

static EspCmdState cmd_state = ESP_CMD_IDLE;
static char        cmd_buf[CMD_BUF_SIZE];
static uint8_t     cmd_pos = 0;

// ===========================================================================
//  Private Helpers
// ===========================================================================

static void UCA1_DrainBuffer(void)
{
    uint8_t byte;
    while ((UCA1IFG & UCTXIFG) && rb_pop(&PCTXBUF, &byte))
        UCA1TXBUF = byte;
}

static void ESP_SendToPC(const char *str)
{
    while (*str)
        rb_push(&PCTXBUF, (uint8_t)(*str++));
    rb_push(&PCTXBUF, '\r');
    rb_push(&PCTXBUF, '\n');
    UCA1IE |= UCTXIE;
}

static void ESP_SetUCA0Baud_115200(void)
{
    UCA0CTLW0  =  UCSWRST;
    UCA0CTLW0 |=  UCSSEL__SMCLK;
    UCA0BRW    =  4;
    UCA0MCTLW  = (0x55u << 8) | UCOS16 | (5u << 4);
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE    |=  UCRXIE;
}

static void ESP_SetUCA0Baud_9600(void)
{
    UCA0CTLW0  =  UCSWRST;
    UCA0CTLW0 |=  UCSSEL__SMCLK;
    UCA0BRW    =  52;
    UCA0MCTLW  = (0x49u << 8) | UCOS16 | (1u << 4);
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE    |=  UCRXIE;
}

// ---------------------------------------------------------------------------
//  UPDATED ESP_HandleCommand
// ---------------------------------------------------------------------------
static void ESP_HandleCommand(void)
{
    if (cmd_pos == 0) return;

    // 1. Check for legacy single-letter commands first
    if (cmd_buf[0] == '^' && cmd_pos == 1) {
        ESP_SendToPC("I'm here");
    }
    else if (cmd_buf[0] == 'F' && cmd_pos == 1) {
        ESP_SetUCA0Baud_115200();
        ESP_SendToPC("115,200");
    }
    else if (cmd_buf[0] == 'S' && cmd_pos == 1) {
        ESP_SetUCA0Baud_9600();
        ESP_SendToPC("9,600");
    }
    // 2. Check for Robot Commands (Starts with a number, e.g., 1234F5678)
    else if (cmd_buf[0] >= '0' && cmd_buf[0] <= '9') 
    {
        uint16_t param1 = 0;
        uint16_t param2 = 0;
        char command_letter = 0;
        uint8_t i = 0;

        // Step A: Extract first number
        while (i < cmd_pos && cmd_buf[i] >= '0' && cmd_buf[i] <= '9') {
            param1 = (param1 * 10) + (cmd_buf[i] - '0');
            i++;
        }

        // Step B: Extract the command letter
        if (i < cmd_pos) {
            command_letter = cmd_buf[i];
            i++;
        }

        // Step C: Extract second number
        while (i < cmd_pos && cmd_buf[i] >= '0' && cmd_buf[i] <= '9') {
            param2 = (param2 * 10) + (cmd_buf[i] - '0');
            i++;
        }

        // Step D: Execute Robot Command
        // Step D: Execute Robot Command (PIN = param1, Duration = param2)
        
        // 1. Check the PIN first!
        if (param1 != 1234) 
        {
            ESP_SendToPC("Robot: Access Denied! Invalid PIN.");
        }
        else 
        {
            // 2. PIN is correct, execute the command using param2 as duration
            switch (command_letter) {
                case 'F': 
                    // Robot_MoveForward(param2); 
                    ESP_SendToPC("Robot: Moving Forward");
                    break;

                case 'B': 
                    // Robot_MoveBackward(param2);
                    ESP_SendToPC("Robot: Moving Backward");
                    break;
                    
                case 'L': 
                    // Robot_TurnLeft(param2);
                    ESP_SendToPC("Robot: Turning Left");
                    break;
                    
                case 'R': 
                    // Robot_TurnRight(param2);
                    ESP_SendToPC("Robot: Turning Right");
                    break;

                default:
                    ESP_SendToPC("Robot: Unknown Command");
                    break;
            }
        }
    }

    // Cleanup buffer for the next command
    cmd_pos = 0;
    memset(cmd_buf, 0, CMD_BUF_SIZE);
}

// ===========================================================================
//  Init_Serial — UCA1 (PC port) at 115,200
// ===========================================================================
void Init_Serial(void)
{
    rb_init(&PCTXBUF);
    rb_init(&PCRXBUF);

    input_pos            = 0;
    memset(input_buffer, 0, INPUT_BUFFER_SIZE);
    transmit_display_timer = 0;
    serial_ui_state      = SERIAL_STATE_WAITING;
    serial_display_active = 0;
    input_active         = 0;
    input_timer          = 0;

    UCA1CTLW0  =  UCSWRST;
    UCA1CTLW0 |=  UCSSEL__SMCLK;       // SMCLK = 8 MHz
    UCA1BRW    =  4;
    UCA1MCTLW  = (0x55u << 8) | UCOS16 | (5u << 4);

    UCA1CTLW0 &= ~UCSWRST;
    UCA1IFG   &= ~(UCRXIFG | UCTXIFG);
    UCA1IFG   |=  UCTXIFG;
    UCA1IE    |=  UCRXIE;
}

// ===========================================================================
//  Init_ESP — UCA0 (IOT port) at 115,200
// ===========================================================================
void Init_ESP(void)
{
    rb_init(&IOTTXBUF);
    rb_init(&IOTRXBUF);

    cmd_state = ESP_CMD_IDLE;
    cmd_pos   = 0;
    memset(cmd_buf, 0, CMD_BUF_SIZE);

    UCA0CTLW0  =  UCSWRST;
    UCA0CTLW0 |=  UCSSEL__SMCLK;
    UCA0BRW    =  4;
    UCA0MCTLW  = (0x55u << 8) | UCOS16 | (5u << 4);

    UCA0CTLW0 &= ~UCSWRST;
    UCA0IFG   &= ~(UCRXIFG | UCTXIFG);
    UCA0IFG   |=  UCTXIFG;
    UCA0IE    |=  UCRXIE;
}

// ===========================================================================
//  ESP_SendATCommand
// ===========================================================================
void ESP_SendATCommand(const char *cmd)
{
    while (*cmd)
        rb_push(&IOTTXBUF, (uint8_t)(*cmd++));
    rb_push(&IOTTXBUF, 0x0D);
    UCA0IE |= UCTXIE;
}

// ===========================================================================
//  ESP_Process — call from main loop
// ===========================================================================
void ESP_Process(void)
{
    uint8_t byte;

    // ------------------------------------------------------------------
    // 1.  PC → IOT  (with ^ command interception and display tracking)
    // ------------------------------------------------------------------
    while (rb_pop(&PCRXBUF, &byte))
    {
        // Echo everything back to PC so the terminal shows what was typed
        rb_push(&PCTXBUF, byte);
        serial_ui_state        = SERIAL_STATE_TRANSMIT;
        transmit_display_timer = 0;

        __disable_interrupt();
        input_timer  = 0;
        input_active = 1;
        __enable_interrupt();

        switch (cmd_state)
        {
            case ESP_CMD_IDLE:
                if (byte == '^')
                {
                    cmd_pos   = 0;
                    cmd_state = ESP_CMD_WAIT;
                }
                else if (byte == '\r' || byte == '\n')
                {
                    // Update LCD display line
                    input_active = 0;
                    input_timer  = 0;
                    serial_display_active = 0;
                    memset(serial_display_line, ' ', 10);
                    memcpy(serial_display_line, input_buffer,
                           (input_pos < 10) ? input_pos : 10);
                    serial_display_line[10] = '\0';
                    serial_display_active   = 1;
                    input_pos = 0;
                    memset(input_buffer, 0, INPUT_BUFFER_SIZE);
                    serial_ui_state = SERIAL_STATE_RECEIVED;

                    // Pass CR through to IOT
                    rb_push(&IOTTXBUF, byte);
                    UCA0IE |= UCTXIE;
                }
                else if (byte == 0x7F || byte == '\b')
                {
                    if (input_pos > 0) input_pos--;
                }
                else
                {
                    if (input_pos < INPUT_BUFFER_SIZE - 1)
                        input_buffer[input_pos++] = byte;
                    // Pass through to IOT
                    rb_push(&IOTTXBUF, byte);
                    UCA0IE |= UCTXIE;
                }
                break;

            case ESP_CMD_WAIT:
                if (byte == '^')
                {
                    cmd_buf[0] = '^';
                    cmd_pos    = 1;
                    ESP_HandleCommand();
                    cmd_state  = ESP_CMD_IDLE;
                }
                else if (byte == 0x0D || byte == 0x0A)
                {
                    cmd_state = ESP_CMD_IDLE;   // Bare '^' — discard
                }
                else
                {
                    cmd_buf[cmd_pos++] = byte;
                    cmd_state = ESP_CMD_BUILD;
                }
                break;

            case ESP_CMD_BUILD:
                if (byte == 0x0D || byte == 0x0A)
                {
                    ESP_HandleCommand();
                    cmd_state = ESP_CMD_IDLE;
                }
                else if (cmd_pos < CMD_BUF_SIZE - 1)
                {
                    cmd_buf[cmd_pos++] = byte;
                }
                break;
        }
    }

    // ------------------------------------------------------------------
    // 2.  IOT → PC  (pass-through)
    // ------------------------------------------------------------------
    while (rb_pop(&IOTRXBUF, &byte))
        rb_push(&PCTXBUF, byte);

    // ------------------------------------------------------------------
    // 3.  Drain UCA1 TX
    // ------------------------------------------------------------------
    UCA1_DrainBuffer();

    // ------------------------------------------------------------------
    // 4.  Display timer state machine
    // ------------------------------------------------------------------
    if (serial_ui_state == SERIAL_STATE_TRANSMIT)
    {
        if (transmit_display_timer >= 10)
        {
            transmit_display_timer = 0;
            serial_ui_state        = SERIAL_STATE_WAITING;
        }
    }
}

// ===========================================================================
//  Serial_TransmitBuffer — SW2 re-sends last received line to PC
// ===========================================================================
void Serial_TransmitBuffer(void)
{
    uint8_t i;
    uint8_t len = 10;

    if (serial_ui_state != SERIAL_STATE_RECEIVED) return;

    while (len > 0 && serial_display_line[len - 1] == ' ') len--;
    if (len == 0) return;

    for (i = 0; i < len; i++)
        rb_push(&PCTXBUF, (uint8_t)serial_display_line[i]);

    rb_push(&PCTXBUF, '\r');
    rb_push(&PCTXBUF, '\n');

    transmit_display_timer = 0;
    serial_ui_state        = SERIAL_STATE_TRANSMIT;
    UCA1IE |= UCTXIE;
}

// ===========================================================================
//  Serial_TimerTick — call from Timer_B0 ISR
// ===========================================================================
void Serial_TimerTick(void)
{
    if (serial_ui_state == SERIAL_STATE_TRANSMIT)
        transmit_display_timer++;

    if (input_active)
    {
        input_timer++;
        if (input_timer >= 25)
        {
            input_active    = 0;
            input_timer     = 0;
            serial_ui_state = SERIAL_STATE_WAITING;
        }
    }
}

// ===========================================================================
//  ESP_TimerTick — call from Timer_B0 ISR
// ===========================================================================
void ESP_TimerTick(void)
{
    // Reserved for future timed IOT operations
}

// ===========================================================================
//  UCA1 ISR  (PC port)
// ===========================================================================
#pragma vector=EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void)
{
    uint8_t byte;
    switch (__even_in_range(UCA1IV, 0x08))
    {
        case USCI_UART_UCRXIFG:
            byte = UCA1RXBUF;
            rb_push(&PCRXBUF, byte);
            break;

        case USCI_UART_UCTXIFG:
            if (rb_pop(&PCTXBUF, &byte))
            {
                UCA1TXBUF = byte;
            }
            else
            {
                UCA1IE  &= ~UCTXIE;
                UCA1IFG |=  UCTXIFG;
            }
            break;
    }
}

// ===========================================================================
//  UCA0 ISR  (IOT port)
// ===========================================================================
#pragma vector=EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void)
{
    uint8_t byte;
    switch (__even_in_range(UCA0IV, 0x08))
    {
        case USCI_UART_UCRXIFG:
            byte = UCA0RXBUF;
            rb_push(&IOTRXBUF, byte);
            break;

        case USCI_UART_UCTXIFG:
            if (rb_pop(&IOTTXBUF, &byte))
            {
                UCA0TXBUF = byte;
            }
            else
            {
                UCA0IE  &= ~UCTXIE;
                UCA0IFG |=  UCTXIFG;
            }
            break;
    }
}