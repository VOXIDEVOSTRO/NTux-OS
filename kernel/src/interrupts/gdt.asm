section .text
global gdt_load
global reload_segments   ; Export this symbol so gdt_load can call it

; --------------------------------------------------------
; NTux-OS: Load the Global Descriptor Table (GDT)
; 
; gdt_load(struct gdt_ptr* gdt_descriptor)
; Input: RDI = pointer to GDTR structure
;
; This function:
; 1. Loads the GDT register with the given pointer
; 2. Calls reload_segments to refresh segment registers (CS, DS, SS, etc.)
; --------------------------------------------------------
gdt_load:
    lgdt [rdi]               ; Load GDTR with pointer provided in RDI
    call reload_segments      ; Reload all segment registers
    ret                       ; Return to caller

; --------------------------------------------------------
; NTux-OS: Reload all segment registers
;
; After loading a new GDT, the CPU still uses old segment registers.
; This routine:
; 1. Updates the Code Segment (CS) using a far return (retfq)
; 2. Updates Data Segment registers (DS, ES, FS, GS, SS)
; --------------------------------------------------------
reload_segments:
    ; Load Code Segment via far return
    push 0x08                ; Selector for kernel code segment (GDT_KERNEL_CS << 3)
    lea rax, [rel .reload_cs] ; Load address of the label where CS will be updated
    push rax
    retfq                    ; Far return to reload CS with new GDT
.reload_cs:
    ; Reload all data segments with kernel data selector
    mov ax, 0x10             ; Selector for kernel data segment (GDT_KERNEL_DS << 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret                       ; Return to caller (now all segments are updated)