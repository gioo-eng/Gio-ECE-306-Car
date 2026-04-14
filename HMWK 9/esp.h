#ifndef ESP_H
#define ESP_H

#include "msp430.h"
#include <stdint.h>
#include <string.h>

// ===========================================================================
//  Ring Buffer  (formerly ringbuffer.h)
// ===========================================================================
#define BUFFER_SIZE  256
#define BUFFER_MASK  (BUFFER_SIZE - 1)

typedef struct {
    volatile uint8_t buffer[BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} RingBuffer;

void    rb_init(RingBuffer *rb);
uint8_t rb_push(RingBuffer *rb, uint8_t data);
uint8_t rb_pop (RingBuffer *rb, uint8_t *dest);

// ===========================================================================
//  Serial Display State  (formerly serial.h)
// ===========================================================================
typedef enum {
    SERIAL_STATE_WAITING,
    SERIAL_STATE_TRANSMIT,
    SERIAL_STATE_RECEIVED
} SerialDisplayState;

// ===========================================================================
//  All Shared Ring Buffers
// ===========================================================================
extern RingBuffer PCTXBUF;    // FRAM → PC     (UCA1 TX)
extern RingBuffer PCRXBUF;    // PC   → FRAM   (UCA1 RX)
extern RingBuffer IOTTXBUF;   // FRAM → ESP32  (UCA0 TX)
extern RingBuffer IOTRXBUF;   // ESP32 → FRAM  (UCA0 RX)

// ===========================================================================
//  Serial Display State Externs  (used by display.c)
// ===========================================================================
extern volatile SerialDisplayState serial_ui_state;
extern volatile unsigned char      serial_display_active;
extern char                        serial_display_line[11];
extern volatile uint8_t            transmit_display_timer;

// ===========================================================================
//  Public API
// ===========================================================================

// UCA1 (PC port) init — call once at startup
void Init_Serial(void);

// Send the last received line back to PC on SW2 press
void Serial_TransmitBuffer(void);

// Call from Timer_B0 ISR
void Serial_TimerTick(void);

// UCA0 (IOT port) init — call once at startup, after Init_Serial()
void Init_ESP(void);

// Call from main loop — handles PC<->IOT bridging and ^ command parsing
void ESP_Process(void);

// Call from Timer_B0 ISR alongside Serial_TimerTick()
void ESP_TimerTick(void);

// Send a NUL-terminated AT command to the ESP32 — appends 0x0D (CR)
void ESP_SendATCommand(const char *cmd);

#endif // ESP_H