global as_movsq
global as_movsd
global as_movsw
global as_movsb

global as_rmovsq
global as_rmovsd
global as_rmovsw
global as_rmovsb

; The only parameters we will need to move are rdx to rcx for the count, 
; otherwise, rdi and rsi are what they should be
as_movsq:
    test rdx, rdx
    jz .end
    cld
    mov rcx, rdx
    rep movsq
.end:
    ret
as_movsd:
    test rdx, rdx
    jz .end
    cld
    mov rcx, rdx
    rep movsd
.end:
    ret
as_movsw:
    test rdx, rdx
    jz .end
    cld
    mov rcx, rdx
    rep movsw
.end:
    ret
as_movsb:
    test rdx, rdx
    jz .end
    cld
    mov rcx, rdx
    rep movsb
.end:
    ret

; reverse copy
as_rmovsq:
    test rdx, rdx
    jz .end
    std
    mov rcx, rdx
    rep movsq
.end:
    ret
as_rmovsd:
    test rdx, rdx
    jz .end
    std
    mov rcx, rdx
    rep movsd
.end:
    ret
as_rmovsw:
    test rdx, rdx
    jz .end
    std
    mov rcx, rdx
    rep movsw
.end:
    ret
as_rmovsb:
    test rdx, rdx
    jz .end
    std
    mov rcx, rdx
    rep movsb
.end:
    ret
