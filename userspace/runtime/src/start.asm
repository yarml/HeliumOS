global _start

extern run

_start:
  call run
  mov rdi, rax
  mov rax, 0
  syscall
