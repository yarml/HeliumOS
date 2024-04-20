global timer_inter_1

extern timer_inter_2

section .text
timer_inter_1:
  cli
  push rax
  push rbx
  push rcx
  push rdx
  
  mov rax, [rsp + 56] ; RSP
  mov rbx, [rsp + 32] ; RIP
  mov rcx, [rsp + 48] ; RFL
  mov rdx, [rsp + 40] ; CS

  push rax ; RSP

  mov rax, [rsp + 72] ; SS

  push rbp
  push rsi
  push rdi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15

  push rbx ; RIP
  push rcx ; RFL
  push dx  ; CS
  push ax  ; SS

  lea rdi, [rsp - 144]

  ; Assure stack is 16 byte aligned before call
  add rsp, 16
  and rsp, 0xFFFFFFFFFFFFFFF0
  add rsp, 8

  call timer_inter_2 ; Never returns