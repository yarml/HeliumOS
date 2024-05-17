global syscall_handle_1

extern syscall_handle_2

section .text
syscall_handle_1:
  cli
  push rax
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
    ; TODO: Use swapgs
    push rax
    push rdx
      mov rax, 1
      cpuid
      shr rbx, 24
    pop rdx
    pop rax

    mov rcx, 0xFFFF840000000000
    shl rbx, 3
    add rcx, rbx
    mov r15, [rcx]

    mov rbp, rsp
    mov rsp, r15
    mov rcx, r10; arg 4
    push rbp
    push rax ; arg 7; syscall number
      call syscall_handle_2
    pop rax
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
  pop rax
  o64 sysret


