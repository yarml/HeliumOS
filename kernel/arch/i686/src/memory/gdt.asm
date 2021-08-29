section .text
global i686_load_gdt:function

i686_load_gdt:
    lgdt [esp+4]
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    jmp  0x08:far_jump
far_jump:
    ret
.end:


; Pseudo code
; [[global]]
; i686_load_gdt(gdtr)
; {
;   lgdt(gdtr) // lgdt instruction
;   %ds, %es, %fs, %gs, %ss = 0x10
;   %cs = 0x08
; }