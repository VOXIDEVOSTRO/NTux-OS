// kernel/include/elf.h
#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>

// ELF64 Header
typedef struct {
    uint32_t   e_magic;     // muss 0x464C457F sein
    uint8_t    e_class;     // 2 = 64-bit
    uint8_t    e_data;      // 1 = little endian
    uint8_t    e_eiversion;
    uint8_t    e_osabi;
    uint8_t    e_abiversion;
    uint8_t    e_pad[7];
    uint16_t   e_type;      // 2 = EXEC
    uint16_t   e_machine;   // 0x3E = x86_64
    uint32_t   e_version;
    uint64_t   e_entry;
    uint64_t   e_phoff;
    uint64_t   e_shoff;
    uint32_t   e_flags;
    uint16_t   e_ehsize;
    uint16_t   e_phentsize;
    uint16_t   e_phnum;
    uint16_t   e_shentsize;
    uint16_t   e_shnum;
    uint16_t   e_shstrndx;
} __attribute__((packed)) elf64_ehdr_t;

// ELF64 Program Header
typedef struct {
    uint32_t   p_type;      // 1 = PT_LOAD
    uint32_t   p_flags;     // 1=exec, 2=write, 4=read
    uint64_t   p_offset;
    uint64_t   p_vaddr;
    uint64_t   p_paddr;
    uint64_t   p_filesz;
    uint64_t   p_memsz;
    uint64_t   p_align;
} __attribute__((packed)) elf64_phdr_t;

#define PT_LOAD    1


void* elf64_load_and_prepare(void *file) ;
void elf64_enter_ring3(uint64_t entry);

#endif