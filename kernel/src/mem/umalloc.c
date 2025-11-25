#include "umalloc.h"
#include "vmm.h"
#include "pmm.h"
#include <drivers/framebuffer/kprint.h>

static uintptr_t user_heap_start = USER_HEAP_START;
static uintptr_t user_heap_end = USER_HEAP_START;
static uintptr_t user_heap_max = USER_HEAP_START + USER_HEAP_INITIAL_SIZE;

static umalloc_block_t *free_list = NULL;

//help function 
static inline size_t align8(size_t size){
    return (size +7) & -7;
}

static void  expand_user_heap(size_t size){
    while (user_heap_end+size>user_heap_max)
    {
        void *phys = pmm_alloc_page();
        if (!phys) {
            kprint("umalloc: out of physical memory!\n");
            return;
        }
        vmm_map_page((void *)user_heap_end,phys,0x7);
        user_heap_end += PAGE_SIZE;
    }
    
}

void umalloc_init(void){
    kprint("initing umalloc\n");
    user_heap_start = USER_HEAP_START;
    user_heap_end = USER_HEAP_START;
    user_heap_max = USER_HEAP_START+ USER_HEAP_INITIAL_SIZE;
    free_list = NULL;
    kprint("umalloc initzialized!\n");
}

void *umalloc (size_t size){
    if(size==0)return NULL;
    size = align8(size);
    umalloc_block_t **prev = &free_list;
    umalloc_block_t *curr = free_list;
    while(curr){
        if(curr->size >= size){
            *prev = curr-> next;
            return (void*)(curr+1);
        }
        prev = &curr->next;
        curr = curr->next;
    }

    expand_user_heap(size+sizeof(umalloc_block_t));
    umalloc_block_t *block = (umalloc_block_t *)user_heap_end;
    block->size = size;
    user_heap_end += size +sizeof(umalloc_block_t);
    return (void*)(block+1);
}

void ufree(void *ptr) {
    if (!ptr) return;

    umalloc_block_t *block = (umalloc_block_t *)ptr - 1;
    block->next = free_list;
    free_list = block;
}