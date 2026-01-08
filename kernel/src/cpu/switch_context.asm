global switch_context

section .text
switch_context:
    ; rdi = &old_rsp
    ; rsi = new_rsp

    mov [rdi], rsp     ; save rsp
    mov rsp, rsi       ; load new rsp
    ret                ; return into new thread
