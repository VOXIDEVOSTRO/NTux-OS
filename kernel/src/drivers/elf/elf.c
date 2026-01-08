// kernel/src/elf.c
#include "elf.h"
#include <libc/string.h>
#include <drivers/framebuffer/kprint.h>
#include <mem/vmm.h>
#include <mem/pmm.h>

#define ELF_MAGIC 0x464C457F
#define PT_LOAD   1

// Deine Flags (passe sie an deine Definition in vmm.h an!)
#define VMM_PRESENT   (1ULL << 0)
#define VMM_WRITE     (1ULL << 1)
#define VMM_USER      (1ULL << 2)
#define VMM_NX        (1ULL << 63)   // falls du No-Execute hast

static int elf64_check(elf64_ehdr_t *hdr)
{
    if (hdr->e_magic != ELF_MAGIC) return 0;
    if (hdr->e_class != 2)         return 0;
    if (hdr->e_data != 1)          return 0;
    if (hdr->e_machine != 0x3E)    return 0;
    if (hdr->e_type != 2)          return 0;
    return 1;
}

void* elf64_load_and_prepare(void *file)
{
    elf64_ehdr_t *ehdr = (elf64_ehdr_t*)file;

    if (!elf64_check(ehdr)) {
        kprintf("[ELF] Invalid ELF header!\n");
        return NULL;
    }

    kprintf("[ELF] Loading userspace ELF @ entry 0x%lx\n", ehdr->e_entry);

    elf64_phdr_t *phdr = (elf64_phdr_t*)((uint8_t*)file + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)
            continue;

        uint64_t vaddr  = phdr[i].p_vaddr & ~0xFFFULL;   // auf Seite ausrichten
        uint64_t offset = phdr[i].p_offset;
        uint64_t filesz = phdr[i].p_filesz;
        uint64_t memsz  = phdr[i].p_memsz;
        int      writable = (phdr[i].p_flags & 2);
        int      executable = (phdr[i].p_flags & 1);

        uint64_t flags = VMM_PRESENT | VMM_USER;
        if (writable)   flags |= VMM_WRITE;
        if (!executable && 0) flags |= VMM_NX;  // nur wenn du NX unterstützt

        kprintf("[ELF]   Mapping 0x%lx → 0x%lx (size %lu KB)%s%s\n",
                (uint64_t)file + offset, vaddr, (memsz + 1023)/1024,
                writable ? " W" : "", executable ? " X" : "");

        for (uint64_t off = 0; off < memsz; off += 0x1000) {
            uint64_t page_vaddr = vaddr + off;
            void *phys = pmm_alloc_page();


            // Mappe die Seite
            vmm_map_page((void*)page_vaddr, phys, flags);

            // Kopiere Daten aus 
            uint64_t src_offset = offset + off;
            uint64_t copy_size = 0x1000;

            if (off >= filesz) {
                copy_size = 0; 
            } else if (off + 0x1000 > filesz) {
                copy_size = filesz - off;
            }

            if (copy_size > 0) {
                memcpy((void*)page_vaddr, (uint8_t*)file + src_offset, copy_size);
            }
        }
    }

    kprintf("[ELF] Userspace ELF loaded – entry = 0x%lx\n", ehdr->e_entry);
    return (void*)ehdr->e_entry;
}

void elf64_enter_ring3(uint64_t entry)
{
    kprintf("[ELF] Entering Ring 3 @ 0x%lx\n", entry);

    __asm__ volatile (
        "mov $0x23, %%ax\n"      
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushq $0x23\n"           
        "pushq %%rsp\n"           
        "pushfq\n"                
        "popq %%rax\n"            
        "orq $0x200, %%rax\n"     
        "pushq %%rax\n"           
        "pushq $0x2B\n"           
        "pushq %0\n"              
        "iretq\n"                 
        : : "r"(entry) : "memory"
    );
}