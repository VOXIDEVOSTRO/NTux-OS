#include "ringbuffer.h"

void rb_init(ringbuffer_t* rb) {
    rb->head = 0;
    rb->tail = 0;
}

int rb_empty(ringbuffer_t* rb) {
    return rb->head == rb->tail;
}

int rb_put(ringbuffer_t* rb, char c) {
    uint32_t next = (rb->head + 1) % RINGBUFFER_SIZE;
    if(next == rb->tail) return 0; // voll
    rb->buffer[rb->head] = c;
    rb->head = next;
    return 1;
}

int rb_get(ringbuffer_t* rb, char* c) {
    if(rb_empty(rb)) return 0;
    *c = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RINGBUFFER_SIZE;
    return 1;
}