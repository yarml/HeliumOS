
section .rodata:
  msg: db "Hello, World from Userspace!", 0

section .text
_start:
  ; output(msg)
  mov rax, 0
  mov rdi, msg
  syscall

  ; exit(42)
  mov rax, 1
  mov rdi, 42
  syscall

  int 32
