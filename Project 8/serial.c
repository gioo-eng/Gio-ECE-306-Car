#include "serial.h"
#include "macros.h"
#include <string.h>

// Declared in display.c
extern volatile unsigned char serial_display_active;
extern char serial_display_line[11];

RingBuffer PCTXBUF = {{0}, 0, 0};
RingBuffer PCRXBUF = {{0}, 0, 0};
SerialMode serialMode = SERIAL_MODE_PC_LOOPBACK;
SerialBaud serialBaud = SERIAL_BAUD_115200;

// Buffer to accumulate incoming characters before writing to the LCD.
// The LCD line is 10 characters wide, so we keep 10 chars + null terminator.
#define LCD_LINE_WIDTH  (10)
static char serial_lcd_buf[LCD_LINE_WIDTH + 1] = "          ";
static uint8_t serial_lcd_pos = 0;

// Drains PCRXBUF and writes incoming characters to display_line[0].
// A newline ('\n') or carriage return ('\r') clears the line and resets the
// cursor. Any other character is appended at the current cursor position.
// Once the line is full, characters scroll left to make room.
// display_changed is set so Display_Process() picks up the update.
static void Serial_DrainBufferToLCD(void)
{
    uint8_t rxByte;

    while (rb_pop(&PCRXBUF, &rxByte))
    {
        if (rxByte == '\n' || rxByte == '\r')
        {
            // Clear line and reset cursor on newline / carriage return
            memset(serial_lcd_buf, ' ', LCD_LINE_WIDTH);
            serial_lcd_buf[LCD_LINE_WIDTH] = '\0';
            serial_lcd_pos = 0;
            memcpy(serial_display_line, serial_lcd_buf, LCD_LINE_WIDTH);
        }
        else
        {
            if (serial_lcd_pos < LCD_LINE_WIDTH)
            {
                serial_lcd_buf[serial_lcd_pos++] = (char)rxByte;
            }
            else
            {
                // Line full — scroll left and append new character at the end
                memmove(serial_lcd_buf, serial_lcd_buf + 1, LCD_LINE_WIDTH - 1);
                serial_lcd_buf[LCD_LINE_WIDTH - 1] = (char)rxByte;
            }

            // Copy into staging buffer and tell display.c to show it on line 0
            memcpy(serial_display_line, serial_lcd_buf, LCD_LINE_WIDTH);
            serial_display_active = 1;
        }
    }
}

static void Serial_DrainBufferToUart(uint16_t baseAddress, RingBuffer *buffer)
{
    uint8_t txByte;

    while (EUSCI_A_UART_getInterruptStatus(baseAddress, EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG))
    {
        if (!rb_pop(buffer, &txByte))
        {
            break;
        }

        EUSCI_A_UART_transmitData(baseAddress, txByte);
    }
}

static void Serial_ReadUartToBuffer(uint16_t baseAddress, RingBuffer *buffer)
{
    while (EUSCI_A_UART_getInterruptStatus(baseAddress, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG))
    {
        if (EUSCI_A_UART_queryStatusFlags(baseAddress, EUSCI_A_UART_RECEIVE_ERROR))
        {
            (void)EUSCI_A_UART_receiveData(baseAddress);
            continue;
        }

        rb_push(buffer, EUSCI_A_UART_receiveData(baseAddress));
    }
}

static void Serial_FillUartBaudConfig(EUSCI_A_UART_initParam *cfg, SerialBaud baud)
{
    cfg->selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    cfg->parity = EUSCI_A_UART_NO_PARITY;
    cfg->msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    cfg->numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    cfg->uartMode = EUSCI_A_UART_MODE;
    cfg->overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    if (baud == SERIAL_BAUD_460800)
    {
        // 8MHz / 460800
        cfg->clockPrescalar = 17;
        cfg->firstModReg = 0;
        cfg->secondModReg = 74;
        cfg->overSampling = 0;
    }
    else
    {
        // 8MHz / 115200
        cfg->clockPrescalar = 4;
        cfg->firstModReg = 5;
        cfg->secondModReg = 85;
        cfg->overSampling = 1;
    }
}

static void Serial_ConfigModule(uint16_t baseAddress,
                                uint8_t gpioPort,
                                uint16_t rxPin,
                                uint16_t txPin,
                                SerialBaud baud)
{
    EUSCI_A_UART_initParam cfg;

    Serial_FillUartBaudConfig(&cfg, baud);

    GPIO_setAsPeripheralModuleFunctionInputPin(gpioPort, rxPin, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(gpioPort, txPin, GPIO_PRIMARY_MODULE_FUNCTION);

    EUSCI_A_UART_disable(baseAddress);
    if (EUSCI_A_UART_init(baseAddress, &cfg) == STATUS_FAIL)
    {
        return;
    }

    EUSCI_A_UART_clearInterrupt(baseAddress,
                                EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG |
                                    EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
#if SERIAL_ONLY_BUILD
    EUSCI_A_UART_disableInterrupt(baseAddress,
                                  EUSCI_A_UART_RECEIVE_INTERRUPT |
                                      EUSCI_A_UART_TRANSMIT_INTERRUPT);
#else
    EUSCI_A_UART_disableInterrupt(baseAddress, EUSCI_A_UART_TRANSMIT_INTERRUPT);
    EUSCI_A_UART_enableInterrupt(baseAddress, EUSCI_A_UART_RECEIVE_INTERRUPT);
#endif
    EUSCI_A_UART_enable(baseAddress);
}

void InitSerialIOT(void)
{
    Serial_ConfigModule(EUSCI_A0_BASE, GPIO_PORT_P1, GPIO_PIN6, GPIO_PIN7, serialBaud);
}

void InitSerialPCLoopback(void)
{
    Serial_ConfigModule(EUSCI_A1_BASE, GPIO_PORT_P4, GPIO_PIN2, GPIO_PIN3, serialBaud);
}

void Init_Serial(void)
{
    rb_init(&PCTXBUF);
    rb_init(&PCRXBUF);

    // Clear the LCD line buffer on init
    memset(serial_lcd_buf, ' ', LCD_LINE_WIDTH);
    serial_lcd_buf[LCD_LINE_WIDTH] = '\0';
    serial_lcd_pos = 0;
    

    if (serialMode == SERIAL_MODE_PC_LOOPBACK)
    {
        InitSerialPCLoopback();
    }
    InitSerialIOT();
}

void Serial_setBaud(SerialBaud baud)
{
    serialBaud = baud;
    Init_Serial();
}

void Serial_setMode(SerialMode mode)
{
    serialMode = mode;
}

void SerialProcess(void)
{
#if SERIAL_ONLY_BUILD
    if (serialMode == SERIAL_MODE_PC_LOOPBACK)
    {
        Serial_ReadUartToBuffer(EUSCI_A1_BASE, &PCRXBUF);
    }
    else
    {
        Serial_ReadUartToBuffer(EUSCI_A0_BASE, &PCRXBUF);
    }
#endif

    // Echo buffered bytes back to PC
    if (serialMode == SERIAL_MODE_PC_LOOPBACK)
    {
        Serial_DrainBufferToUart(EUSCI_A1_BASE, &PCTXBUF);
    }

    // Display received bytes on LCD line 0
    Serial_DrainBufferToLCD();
}

#pragma vector=EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void)
{
    uint8_t rxByte;

    switch (__even_in_range(UCA0IV, 0x08))
    {
    case 0:
        break;
    case 2: // RXIFG
        if (EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_ERROR))
        {
            (void)EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
            break;
        }
        rxByte = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
        rb_push(&PCRXBUF, rxByte);
        break;
    case 4: // TXIFG
        EUSCI_A_UART_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
        break;
    default:
        break;
    }
}

#pragma vector=EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void)
{
    uint8_t rxByte;

    switch (__even_in_range(UCA1IV, 0x08))
    {
    case 0:
        break;
    case 2: // RXIFG
        if (EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_ERROR))
        {
            (void)EUSCI_A_UART_receiveData(EUSCI_A1_BASE);
            break;
        }
        rxByte = EUSCI_A_UART_receiveData(EUSCI_A1_BASE);
        rb_push(&PCRXBUF, rxByte);  // Goes to LCD via SerialProcess
        rb_push(&PCTXBUF, rxByte);  // Echo back to PC
        EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
        break;
    case 4: // TXIFG
        EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
        break;
    default:
        break;
    }
}