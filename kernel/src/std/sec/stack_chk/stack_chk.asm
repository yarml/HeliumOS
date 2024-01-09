global __stack_chk_fail

extern __stack_chk_stack
extern __stack_chk_lock
extern __stack_chk_fail_p2
extern as_sys_stop

section .text
__stack_chk_fail:
    ; Check the lock
    lea rdx, [__stack_chk_lock]
    xor rax, rax
    mov ecx, 1
    lock cmpxchg DWORD [rdx], ecx

    ; If RAX is set to 1, the value failed to be loaded, lock already taken
    cmp rax, 1
    je as_sys_stop ; Simply stop()

    ; Lock is ours now
    lea rax, [__stack_chk_stack + 1024]
    mov rsp, rax
    call __stack_chk_fail_p2
    jmp as_sys_stop
