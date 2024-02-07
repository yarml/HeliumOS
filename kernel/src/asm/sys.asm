global as_sys_stop
global as_event_loop
global as_syscall_handle

extern syscall

section .text

as_sys_stop:
  cli
.loop:
  hlt
  jmp .loop

as_event_loop:
  sti
.loop:
  hlt
  jmp .loop

as_syscall_handle:
  push rcx
  push r11
  call syscall
  pop r11
  pop rcx
  sysret
