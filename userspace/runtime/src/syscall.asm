global dosyscall

section .text
; dosyscall(syscall_num, args...)
dosyscall:
  mov r10, rcx
  syscall
  ret
