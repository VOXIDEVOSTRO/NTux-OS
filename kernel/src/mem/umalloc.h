//this class is for userspaces mem alloc

#ifndef UMALLOC_H
#define UMALLOC_H

#include <stddef.h>
#include <stdint.h>

#define USER_HEAP_START 0x00400000
#define USER_HEAP_INITIAL_SIZE 0x1000000

typedef struct umalloc_block{
    size_t size;
    struct  umalloc_block * next;
}umalloc_block_t;

void umalloc_init(void);
void *umalloc(size_t size);
void ufree(void*ptr);

#endif