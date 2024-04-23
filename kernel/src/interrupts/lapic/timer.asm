global timer_inter_1

extern timer_inter_2

section .text
timer_inter_1:
  cli

  ; Elements in reverse order to that in TaskProcState
  push rdx
  push rcx
  push rbx
  push rax
  push r15
  
  mov rax, [rsp + 40] ; RIP
  mov rbx, [rsp + 48] ; CS
  mov rcx, [rsp + 56] ; RFL
  mov rdx, [rsp + 64] ; RSP
  mov r15, [rsp + 72] ; SS

  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rdi
  push rsi
  push rbp
  
  push rdx ; RSP
  push r15 ; SS
  push rbx ; CS
  push rcx ; RFL
  push rax ; RIP

  mov rdi, rsp

  ; Assure stack is 16 byte aligned before call
  sub rsp, 16
  and rsp, 0xFFFFFFFFFFFFFFF0
  sub rsp, 8

  call timer_inter_2 ; Never returns