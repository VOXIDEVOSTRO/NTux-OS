#include "kmalloc.h"
#include "vmm.h"
#include "pmm.h"
#include <drivers/framebuffer/kprint.h>

#define KERNEL_HEAP_START 0xFFFF800001000000ULL
#define KERNEL_HEAP_INITIAL_SIZE 0x100000 

typedef struct kmalloc_block {
    size_t size;               
    struct kmalloc_block *next; 
} kmalloc_block_t;

static uintptr_t heap_start = KERNEL_HEAP_START;
static uintptr_t heap_end   = KERNEL_HEAP_START;
static uintptr_t heap_max   = KERNEL_HEAP_START + KERNEL_HEAP_INITIAL_SIZE;

static kmalloc_block_t *free_list = NULL;

static inline size_t align8(size_t size) {
    return (size + 7) & ~7;
}

static void expand_heap(size_t size) {
    while (heap_end + size > heap_max) {
        void *phys = pmm_alloc_page();
        if (!phys) {
            kprint("kmalloc: out of physical memory!\n");
            return;
        }

        vmm_map_page((void *)heap_end, phys, 0x3); 
        heap_end += PAGE_SIZE;
    }
}


void kmalloc_init(void) {
    kprint("Initializing kernel heap...\n");
    heap_start = KERNEL_HEAP_START;
    heap_end   = KERNEL_HEAP_START;
    heap_max   = KERNEL_HEAP_START + KERNEL_HEAP_INITIAL_SIZE;
    free_list  = NULL;
    kprint("Kernel heap initialized.\n");
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = align8(size); 
    kmalloc_block_t **prev = &free_list;
    kmalloc_block_t *curr = free_list;
    while (curr) {
        if (curr->size >= size) {
            *prev = curr->next; 
            return (void *)(curr + 1); 
        }
        prev = &curr->next;
        curr = curr->next;
    }
    expand_heap(size + sizeof(kmalloc_block_t));
    kmalloc_block_t *block = (kmalloc_block_t *)heap_end;
    block->size = size;
    heap_end += size + sizeof(kmalloc_block_t);

    return (void *)(block + 1);
}

void kfree(void *ptr) {
    if (!ptr) return;

    kmalloc_block_t *block = (kmalloc_block_t *)ptr - 1; 
    block->next = free_list;  
    free_list = block;
}
