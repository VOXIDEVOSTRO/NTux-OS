[BITS 64]

global interrupts_enable
global interrupts_disable
global interrupts_are_enabled

section .text

; --------------------------------------------------
; void interrupts_enable()
; Executes STI
; --------------------------------------------------
interrupts_enable:
    sti
    ret

; --------------------------------------------------
; void interrupts_disable()
; Executes CLI
; --------------------------------------------------
interrupts_disable:
    cli
    ret

; --------------------------------------------------
; int interrupts_are_enabled()
; Returns 1 if IF flag in RFLAGS is set, else 0
; --------------------------------------------------
interrupts_are_enabled:
    pushfq          ; push RFLAGS to stack
    pop rax         ; pop into RAX
    shr rax, 9      ; move bit 9 (IF flag) into lowest bit
    and rax, 1      ; mask all but lowest bit
    ret
