#include "pmm.h"

static uint8_t *bitmap = 0;
static size_t bitmap_size = 0;
static size_t total_pages = 0;
static size_t free_pages = 0;
static uintptr_t memory_start = 0;

static inline void set_bit(size_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(size_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int test_bit(size_t bit) {
    return (bitmap[bit / 8] >> (bit % 8)) & 1;
}

void pmm_init(struct limine_memmap_response *memmap) {
    size_t highest_addr = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        uintptr_t end = entry->base + entry->length;
        if (end > highest_addr) {
            highest_addr = end;
        }
    }

    total_pages = highest_addr / PAGE_SIZE;
    bitmap_size = total_pages / 8 + 1;

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == USABLE_MEMORY && entry->length >= bitmap_size) {
            bitmap = (uint8_t *)(uintptr_t)entry->base;
            entry->base += bitmap_size;
            entry->length -= bitmap_size;
            break;
        }
    }

    for (size_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0xFF;
    }

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == USABLE_MEMORY) {
            uintptr_t addr = entry->base;
            while (addr + PAGE_SIZE <= entry->base + entry->length) {
                size_t page = addr / PAGE_SIZE;
                clear_bit(page);
                free_pages++;
                addr += PAGE_SIZE;
            }
        }
    }

    memory_start = (uintptr_t)bitmap + bitmap_size;
}

void *pmm_alloc_page(void) {
    for (size_t i = 0; i < total_pages; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            free_pages--;
            return (void *)(i * PAGE_SIZE);
        }
    }
    return 0; 
}

void pmm_free_page(void *addr) {
    size_t page = (uintptr_t)addr / PAGE_SIZE;
    if (test_bit(page)) {
        clear_bit(page);
        free_pages++;
    }
}

size_t pmm_get_free_memory(void) {
    return free_pages * PAGE_SIZE;
}

uint32_t pmm_get_total_memory_32(struct limine_memmap_response *memmap) {
    uint64_t total = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total += entry->length;
        }
    }


    return (uint32_t)total;
}