#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "msp430.h"
#include <stdint.h>

#define BUFFER_SIZE 256
#define BUFFER_MASK (BUFFER_SIZE - 1)

typedef struct {
    volatile uint8_t buffer[BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} RingBuffer;

void rb_init(RingBuffer *rb);
uint8_t rb_push(RingBuffer *rb, uint8_t data);
uint8_t rb_pop(RingBuffer *rb, uint8_t *dest);

#endif
