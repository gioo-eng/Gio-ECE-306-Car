#include "ringbuffer.h"
#include <string.h>

void rb_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    memset(rb->buffer, 0, BUFFER_SIZE);
}

static inline uint8_t rb_is_full(RingBuffer *rb) {
    return (((rb->head + 1) & BUFFER_MASK) == rb->tail);
}

static inline uint8_t rb_is_empty(RingBuffer *rb) {
    return (rb->head == rb->tail);
}

uint8_t rb_push(RingBuffer *rb, uint8_t data) {
    uint8_t next = (rb->head + 1) & BUFFER_MASK;

    if (next == rb->tail) {
        //  BUFFER FULL  DROP NEW DATA
        return 0;
    }

    rb->buffer[rb->head] = data;
    rb->head = next;
    return 1;
}

uint8_t rb_pop(RingBuffer *rb, uint8_t *dest) {
    if (rb_is_empty(rb)) {
        return 0;
    }

    *dest = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & BUFFER_MASK;
    return 1;
}
