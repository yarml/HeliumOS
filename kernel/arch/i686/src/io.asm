global i686_outb
global i686_outw
global i686_outdw

global i686_inb
global i686_inw
global i686_indw

i686_outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret

i686_outw:
    mov ax, [esp + 8]
    mov dx, [esp + 4]
    out dx, ax
    ret

i686_outdw:
    mov eax, [esp + 8]
    mov dx, [esp + 4]
    out dx, eax
    ret

i686_inb:
    mov dx, [esp + 4]
    in  al, dx    
    ret

i686_inw:
    mov dx, [esp + 4]
    in  ax, dx
    ret

i686_indw:
    mov dx, [esp + 4]
    in  eax, dx
    ret
