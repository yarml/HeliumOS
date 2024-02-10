global _start

extern main

section .text
_start:
  call main

  mov rsi, rax
  mov rdi, 0x10
  syscall