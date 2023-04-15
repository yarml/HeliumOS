global module_init

extern putchar

section .text
module_init:
  mov rdi, 'c'
  call putchar
  mov rax, 42
  ret