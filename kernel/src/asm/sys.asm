global as_sys_stop
global as_event_loop
global as_syscall_handle

extern syscall

section .text

as_sys_stop:
  cli
.loop:
  hlt
  jmp .loop

as_event_loop:
  sti
.loop:
  hlt
  jmp .loop

as_syscall_handle:
  push rbx
  push rcx
  push rdx
  push rbp
  push rdi
  push rsi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
    ; We need to get the location of the kernel stack for the current processor
    ; Without using the stack, this is done through the stack table
    ; Put by the kernel init at 0xFFFF840000000000
    push rdx
      mov rax, 1
      cpuid
      shr rbx, 24
    pop rdx

    mov rax, 0xFFFF840000000000
    shl rbx, 3
    add rax, rbx
    mov r15, [rax]

    mov rbp, rsp
    mov rsp, r15
    mov rcx, r10; arg 4
    push rbp
      call syscall
    pop rsp
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rsi
  pop rdi
  pop rbp
  pop rdx
  pop rcx
  pop rbx
  o64 sysret
