global timer_inter_1

extern timer_inter_2

section .text
timer_inter_1:
  ; In previous versions this just used whatever stack was there
  ; This shouldn't be the case, like syscall, we need to use the kernel stack
  ; Gonna copy paste, I need to get familiar with NASM's macros
  ; The assumption we can make is that we are not in a double interrupt situation (hopefully???)
  ; We can't know if we interrupted user code or kernel code
  ; But if we did interrupt kernel code, it was the event loop(hopefully, again), which we can throw away
  ; The important part is that, unlike syscall, we don't know if we need to save the processor state or not
  cli
  ; I may or may not have stolen this from Redox
  swapgs
  mov gs:[0], rsp ; Save user RSP
  mov rsp, gs:[8] ; Load kernel RSP
  ; Now we're in kernel stack
  ; Push current CPU state
  push rdx
  push rcx
  push rbx
  push rax
  push r15
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
  call timer_inter_2 ; Never returns