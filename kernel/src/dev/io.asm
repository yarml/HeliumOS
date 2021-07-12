global a_outb
global a_outw
global a_inb
global a_inw

section .text

; uint8_t inb(uint16_t port);
a_inb:
    mov dx, [esp + 4] ; Port
    in  al, dx    
    ret
; void outb(uint16_t port, uint8_t data);
a_outb:
    mov al, [esp + 8] ; Data
    mov dx, [esp + 4] ; Port
    out dx, al
    ret

; void outw(uint16_t port, uint16_t data);
a_outw:
    mov ax, [esp + 8] ; Data
    mov dx, [esp + 4] ; Port
    out dx, ax
    ret
; uint16_t inw(uint16_t port);
a_inw:
    mov dx, [esp + 4] ; Port
    in  ax, dx
    ret
