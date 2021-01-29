global outb
global outw
global inb
global inw

; uint8_t inb(uint16_t port);
inb:
    mov dx, [esp + 4] ; Port
    in  al, dx
    %include "debug/debug.asm"
    
    ret

; uint16_t inb(uint16_t port);
inw:
    mov dx, [esp + 4] ; Port
    in  ax, dx
    ret

; void outb(uint16_t port, uint8_t data);
outb:
    mov al, [esp + 8] ; Data
    mov dx, [esp + 4] ; Port
    out dx, al
    ret
; void outw(uint16_t port, uint16_t data);
outw:
    mov ax, [esp + 8] ; Data
    mov dx, [esp + 4] ; Port
    out dx, ax
    ret