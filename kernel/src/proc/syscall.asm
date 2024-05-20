global syscall_handle_1

extern syscall_handle_2

section .text
syscall_handle_1:
  cli
  ; I may or may not have stolen this from Redox
  swapgs
  mov gs:[0], rsp ; Save user RSP
  mov rsp, gs:[8] ; Load kernel RSP
  ; Now we're in kernel stack
  ; Push current CPU state
  push rdx
  push rcx ; Modified by syscall, but user knows that
  push rbx
  push rax
  push r15
  push r14
  push r13
  push r12
  push r11 ; Modified by syscall, but user knows that
  push r10
  push r9
  push r8
  push rdi
  push rsi
  push rbp
  
  ; Push user RSP
  mov rax, gs:[0]
  push rax

  push 0x1B ; SS should have been 0x1B
  push 0x23 ; CS should have been 0x23
  push r11 ; user RFL
  push rcx ; user RIP

  mov rdi, rsp

  ; Ensure stack is 16 byte aligned before call
  sub rsp, 16
  and rsp, 0xFFFFFFFFFFFFFFF0
  sub rsp, 8

  swapgs ; Last revert KGSBASE
  call syscall_handle_2 ; This guy takes care of returning to userspace


