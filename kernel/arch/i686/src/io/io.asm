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

; Pseudo code
; [[global]]
; i686_outb(ioadr, byte)
; {
;   out(ioadr, byte) // out instruction
; }
; [[global]]
; i686_outw(ioadr, word)
; {
;   out(ioadr, word) // out instruction
; }
; [[global]]
; i686_outdw(ioadr, dword)
; {
;   out(ioadr, dword) // out instruction
; }

; [[global]]
; i686_inb(ioadr) : byte
; {
;   return in(ioadr) // in instruction
; }
; [[global]]
; i686_inw(ioadr) : word
; {
;   return in(ioadr) // in instruction
; }
; [[global]]
; i686_indw(ioadr) : dword
; {
;   return in(ioadr) // in instruction
; }