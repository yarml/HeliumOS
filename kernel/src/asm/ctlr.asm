global as_scr3
global as_rlcr3

as_scr3:
    mov rax, cr3
    ret

as_rlcr3:
    mov rax, cr3
    mov cr3, rax
    ret
