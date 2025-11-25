#include "vmm.h"
#include "pmm.h"
#include <drivers/framebuffer/kprint.h>

static vmm_page_table_t *current_page_table = NULL;
static uintptr_t kernel_heap_end  = KERNEL_HEAP_START;

void *my_memset(void *ptr, int value, size_t num) {
    unsigned char *p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void vmm_init(struct limine_memmap_response* memap){
    kprint("initing vmm/pmm\n");
    kprint("init pmm\n");
    pmm_init(memap);
    kprint("pmm init comppletetd during pmm test\n");
    kprint("allocating 2 pages for test\n");
    void *page1 = pmm_alloc_page();
    void *page2 = pmm_alloc_page();
    kprint("pages were allocated succesfully freeing pages \n");
    pmm_free_page(page1);
    kprint("pages freed from pmm making vmm page table for kernel\n");

    size_t entries_per_table = PAGE_SIZE / sizeof(pte_t);
    current_page_table = (vmm_page_table_t*) pmm_alloc_page();
    kprint("making memset for vmm \n");
    my_memset(current_page_table,0,sizeof(vmm_page_table_t));
    kprint("making page for kernel only\n");
    current_page_table->entries =(pte_t *)pmm_alloc_page();
    my_memset(current_page_table->entries,0,PAGE_SIZE);
    kprint("was succesfull contining vmm init\n");
    kprint("kernel addres init\n");

    for (size_t i = 0; i < entries_per_table; i++) {
        set_pte(&current_page_table->entries[i], i * PAGE_SIZE, 0x3); 
    }
}

void *vmm_alloc_page(void){
    if (!current_page_table) return NULL;

    void *phys_addr = pmm_alloc_page();
    if (!phys_addr) return NULL;

    my_memset(phys_addr, 0, PAGE_SIZE);

    uintptr_t virt_addr = kernel_heap_end;
    kernel_heap_end += PAGE_SIZE;

    if (vmm_map_page((void*)virt_addr, phys_addr, 0x3) == NULL) {
        pmm_free_page(phys_addr);
        return NULL;
    }

    return (void *)virt_addr;
}

void vmm_free_page(void* addr){
    if (!current_page_table || addr == NULL) return;

    size_t entries_per_table = PAGE_SIZE / sizeof(pte_t);
    size_t virt_page = (uintptr_t)addr / PAGE_SIZE;
    if (virt_page >= entries_per_table) return;

    pte_t *entry = &current_page_table->entries[virt_page];
    uintptr_t phys = get_phys_addr_from_pte(entry);
    if (phys == 0) return;

    set_pte(entry, 0, 0);

    pmm_free_page((void*)phys);
}

void* vmm_map_page(void *virt,void * phys,uint64_t flags){
    if (!current_page_table || virt == NULL || phys == NULL) return NULL;

    size_t entries_per_table = PAGE_SIZE / sizeof(pte_t);
    size_t virt_page = (uintptr_t)virt / PAGE_SIZE;
    if (virt_page >= entries_per_table) return NULL;

    set_pte(&current_page_table->entries[virt_page], (uintptr_t)phys, flags); 

    return virt;
}


void vmm_unmap_page(void * virt){
    if (!current_page_table || virt == NULL) return;

    size_t entries_per_table = PAGE_SIZE / sizeof(pte_t);
    size_t virt_page = (uintptr_t)virt / PAGE_SIZE;
    if (virt_page >= entries_per_table) return;

    set_pte(&current_page_table->entries[virt_page],0,0);

    return;
}

static inline void set_pte(pte_t *entry, uintptr_t phys_addr, uint64_t flags) {
    *entry = (phys_addr & ~0xFFF) | (flags & 0xFFF); 
}

static inline uintptr_t get_phys_addr_from_pte(pte_t *entry) {
    return *entry & ~0xFFF;
}


void vmm_load_page_table(vmm_page_table_t *page_table) {
    
}

void* vmm_alloc_user_pages(size_t count) {
    if (count == 0 || !current_page_table) return NULL;

    void* first_virt = NULL;
    for (size_t i = 0; i < count; ++i) {
        void* phys = pmm_alloc_page();
        if (!phys) {
            if (first_virt) {
                for (size_t j = 0; j < i; ++j) {
                    vmm_free_page((uint8_t*)first_virt + j * PAGE_SIZE);
                }
            }
            return NULL;
        }
        my_memset(phys, 0, PAGE_SIZE);

        uintptr_t virt = kernel_heap_end;
        kernel_heap_end += PAGE_SIZE;

        if (vmm_map_page((void*)virt, phys, 0x7) == NULL) {
            pmm_free_page(phys);
            if (first_virt) {
                for (size_t j = 0; j < i; ++j) {
                    vmm_free_page((uint8_t*)first_virt + j * PAGE_SIZE);
                }
            }
            return NULL;
        }

        if (!first_virt) first_virt = (void*)virt;
    }
    return first_virt;
}

void *vmm_alloc_stack(size_t pages) {
    if (pages == 0 || !current_page_table) return NULL;

    size_t total = pages + 1;
    void *base = vmm_alloc_user_pages(total);
    if (!base) return NULL;
    vmm_unmap_page(base);
    return (uint8_t*)base + total * PAGE_SIZE;
}

void vmm_free_pages_region(void *base, size_t count) {
    if (!base || count == 0 || !current_page_table) return;

    for (size_t i = 0; i < count; ++i) {
        void *page_virt = (uint8_t*)base + i * PAGE_SIZE;
        vmm_free_page(page_virt);
    }
}