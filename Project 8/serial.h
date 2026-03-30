#ifndef SERIAL_H
#define SERIAL_H

#include "msp430.h"
#include "ports.h"
#include <driverlib.h>
#include "ringbuffer.h"

extern RingBuffer PCTXBUF;
extern RingBuffer PCRXBUF;

typedef enum serial_mode {
    SERIAL_MODE_PC_LOOPBACK,
    SERIAL_MODE_IOT
} SerialMode;

typedef enum serial_baud {
    SERIAL_BAUD_115200 = 115200,
    SERIAL_BAUD_460800 = 460800
} SerialBaud;

void InitSerialPCLoopback(void); //Initializes UCA1 to be used for UART

void InitSerialIOT(void); //Initializes UCA0 to be used for UART

void Init_Serial(void);

void Serial_setBaud(SerialBaud baud);

void Serial_setMode(SerialMode mode);

void SerialProcess(void);

extern SerialMode serialMode;
extern SerialBaud serialBaud;

#endif //SERIAL_H