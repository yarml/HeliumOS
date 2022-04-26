global as_scasb
global as_scasw
global as_scasd
global as_scasq

global as_nscasb
global as_nscasw
global as_nscasd
global as_nscasq

as_scasb:
    mov rax, rsi
    mov rcx, rdx
    repne scasb
    mov rax, rcx
    ret

as_scasw:
    mov rax, rsi
    mov rcx, rdx
    repne scasw
    mov rax, rcx
    ret

as_scasd:
    mov rax, rsi
    mov rcx, rdx
    repne scasd
    mov rax, rcx
    ret

as_scasq:
    mov rax, rsi
    mov rcx, rdx
    repne scasq
    mov rax, rcx
    ret


as_nscasb:
    mov rax, rsi
    mov rcx, rdx
    repe scasb
    mov rax, rcx
    ret

as_nscasw:
    mov rax, rsi
    mov rcx, rdx
    repe scasw
    mov rax, rcx
    ret

as_nscasd:
    mov rax, rsi
    mov rcx, rdx
    repe scasd
    mov rax, rcx
    ret

as_nscasq:
    mov rax, rsi
    mov rcx, rdx
    repe scasq
    mov rax, rcx
    ret
