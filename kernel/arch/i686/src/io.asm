section .text

global i686_outb:function
global i686_outw:function
global i686_outdw:function

global i686_inb:function
global i686_inw:function
global i686_indw:function

i686_outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret
.end:

i686_outw:
    mov ax, [esp + 8]
    mov dx, [esp + 4]
    out dx, ax
    ret
.end:

i686_outdw:
    mov eax, [esp + 8]
    mov dx, [esp + 4]
    out dx, eax
    ret
.end:

i686_inb:
    mov dx, [esp + 4]
    in  al, dx    
    ret
.end:

i686_inw:
    mov dx, [esp + 4]
    in  ax, dx
    ret
.end:

i686_indw:
    mov dx, [esp + 4]
    in  eax, dx
    ret
.end:
