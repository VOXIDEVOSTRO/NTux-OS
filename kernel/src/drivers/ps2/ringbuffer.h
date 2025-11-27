#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#define RINGBUFFER_SIZE 256

typedef struct {
    char buffer[RINGBUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} ringbuffer_t;

void rb_init(ringbuffer_t* rb);
int rb_put(ringbuffer_t* rb, char c);
int rb_get(ringbuffer_t* rb, char* c);
int rb_empty(ringbuffer_t* rb);

#endif // RINGBUFFER_H