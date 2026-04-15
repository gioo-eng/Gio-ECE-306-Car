#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define BUF_LEN  32    // Must hold a full +IPD frame: "+IPD,0,XX:^1234DNNNN" + null
#define RING_SZ  128   // Must be a power of 2

void         uart_init(void);
void         uart_set_baud(void);
void         uart_send_buf(const char *src);
uint8_t      uart_read_frame(char *dst);
const char  *uart_get_last_frame(void);

/* Register a byte-level hook called inside the UCA0 RX ISR for every byte
   received from the IOT module.  Pass NULL to unregister.
   Used by Debug_PC_Init() — do not call from application code.          */
void         uart_set_iot_rx_hook(void (*hook)(char));

#endif // SERIAL_H