#include "ringbuffer.h"
#include <string.h>

void rb_init(RingBuffer* rb) {
    rb->head = 0;
    rb->tail = 0;
    memset(rb->buffer, 0, sizeof(uint8_t) * BUFFER_SIZE);
}

inline void rb_push(RingBuffer* rb, uint8_t data) {
    rb->buffer[rb->head] = data;
    uint8_t next = (rb->head + 1) & BUFFER_MASK;

    // BUG FIX: When the buffer is full (next == tail), the old code set
    // head = next (== tail), making the buffer appear *empty* on the next
    // rb_pop call and losing all buffered data.  The correct behaviour is to
    // advance tail so the oldest byte is evicted and head can safely move to
    // next without colliding with tail.
    if (next == rb->tail) {
        rb->tail = (rb->tail + 1) & BUFFER_MASK;
    }

    rb->head = next;
}

inline uint8_t rb_pop(RingBuffer* rb, uint8_t* dest) {
    if (rb->head == rb->tail) {
        return 0;
    }

    *dest = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & BUFFER_MASK;
    return 1;
}