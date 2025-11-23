#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#define PAGE_SIZE 4096
#define USABLE_MEMORY LIMINE_MEMMAP_USABLE

void pmm_init(struct limine_memmap_response *memmap);
void *pmm_alloc_page(void);
void pmm_free_page(void *addr);
size_t pmm_get_free_memory(void);
uint32_t pmm_get_total_memory_32(struct limine_memmap_response *memmap) ;

#endif 