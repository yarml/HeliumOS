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
  swapgs
  mov gs:[0], rsp ; Save RSP before these fuckeries
  ; mov rsp, gs:[8] ; Load kernel RSP, if we were in the kernel before, fuck all
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
  
  ; Push RSP before timer
  mov rax, gs:[0]
  ; RAX is now RSP before the fuckeries
  mov rbx, [rax + 24]
  ; RBX is now RSP before the timer
  push rbx

  ; The processor state is only saved if we came from user mode, so we can assume CS and SS
  push 0x1B ; SS should have been 0x1B
  push 0x23 ; CS should have been 0x23
  ; Now we will move old RFL into r11, and old RIP into RCX (simulating a syscall)
  ; (Just so that I don't have to modify the 2 pushes following)
  ; RAX contains RSP before the fuckeries
  mov r11, [rax + 16]
  mov rcx, [rax + 00]

  push r11 ; user RFL
  push rcx ; user RIP

  ; Hopefully this is good

  mov rdi, rsp

  ; Ensure stack is 16 byte aligned before call
  sub rsp, 16
  and rsp, 0xFFFFFFFFFFFFFFF0
  sub rsp, 8

  swapgs ; Last revert KGSBASE
  call timer_inter_2 ; Never returns