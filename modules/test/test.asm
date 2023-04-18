global module_init

extern printd

section .text
module_init:
  mov rdi, test_msg
  mov rdx, printd
  call printd
  mov rax, 42
  ret

section .rodata:
test_msg: db "Hello kernel module!", 0x0A, 0x00
