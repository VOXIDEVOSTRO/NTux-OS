[BITS 64]

global irq_stub_table

extern irq_handler_c   ; C-Funktion, die wir gleich schreiben

section .text

; ---- IRQ STUBS ----
; Jede IRQ-Stubs speichert Register, ruft C-Funktion auf und schickt PIC ACK.

irq_stub_table:
%assign i 0
%rep 16
    dq irq_stub_%+i
%assign i i+1
%endrep

%macro IRQ_STUB 1
global irq_stub_%1
irq_stub_%1:
    ; Push IRQ number
    cli
    push qword %1  ; Push IRQ Nummer auf den Stack

    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp          ; Argument: pointer to saved regs
    call irq_handler_c    ; Call C-IRQ-Handler

    ; Restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 8            ; Entferne IRQ Nummer vom Stack

    sti
    iretq                 ; Rückkehr von Interrupt
%endmacro

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15