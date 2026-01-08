[BITS 64]

;--------------------------
; NTux OS - I/O Functions for Limine (x86_64)
; This file implements basic I/O port operations for x86_64
;--------------------------

global inb         ; Declare inb function (input byte)
global outb        ; Declare outb function (output byte)
global inw         ; Declare inw function (input word)
global outw        ; Declare outw function (output word)
global inl         ; Declare inl function
global outl        ; Decleare outl function
global ind
global outd

section .text

; uint8_t inb(uint16_t port)
; This function reads a byte from the specified I/O port
inb:
    and rdi, 0xFFFF       ; Mask to get the lower 16 bits of rdi (port)
    mov dx, di            ; Move the lower 16 bits of rdi (port) into dx
    in  al, dx            ; Read a byte from the port into al (8-bit value)
    ret                   ; Return, value is in al (the lower 8 bits of the return value)

; void outb(uint16_t port, uint8_t value)
; This function writes a byte to the specified I/O port
outb:
    and rdi, 0xFFFF       ; Mask to get the lower 16 bits of rdi (port)
    mov dx, di            ; Move the lower 16 bits of rdi (port) into dx
    mov al, sil           ; Move value from rsi (2nd argument) into al (8-bit value)
    out dx, al            ; Write byte in al to the port at dx
    ret                   ; Return

; uint16_t inw(uint16_t port)
; This function reads a word (2 bytes) from the specified I/O port
inw:
    and rdi, 0xFFFF       ; Mask to get the lower 16 bits of rdi (port)
    mov dx, di            ; Move the lower 16 bits of rdi (port) into dx
    in  ax, dx            ; Read 2 bytes from the port into ax (16-bit word)
    ret                   ; Return, value is in ax (the lower 16 bits of the return value)

; void outw(uint16_t port, uint16_t value)
; This function writes a word (2 bytes) to the specified I/O port
outw:
    and rdi, 0xFFFF       ; Mask to get the lower 16 bits of rdi (port)
    mov dx, di            ; Move the lower 16 bits of rdi (port) into dx
    mov ax, si            ; Move value from rsi (2nd argument) into ax (16-bit value)
    out dx, ax            ; Write word in ax to the port at dx
    ret                   ; Return
; uint32_t inl(uint16_t port)   ← 32-Bit reading
inl:
    mov dx, di          
    in  eax, dx         
    ret

; void outl(uint16_t port, uint32_t value)   ← 32-Bit writing
outl:
    mov dx, di          
    mov eax, esi       
    out dx, eax         
    ret
; uint32_t ind(uint16_t port)
; Diese Funktion liest ein 32-Bit Wort (4 Bytes) vom angegebenen I/O-Port
ind:
    mov dx, di          ; Portadresse in dx
    in  eax, dx         ; Lese 32-Bit-Wert vom Port in eax
    ret                 ; Rückgabe, der Wert befindet sich in eax (32-Bit Wert)

; void outd(uint16_t port, uint32_t value)
; Diese Funktion schreibt ein 32-Bit Wort (4 Bytes) zum angegebenen I/O-Port
outd:
    mov dx, di          ; Portadresse in dx
    mov eax, esi        ; Der Wert, den wir schreiben wollen, ist in esi
    out dx, eax         ; Schreibe den 32-Bit-Wert in eax zum Port in dx
    ret                 ; Rückgabe