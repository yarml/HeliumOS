global as_sys_stop

section .text

as_sys_stop:
  cli
.loop:
  hlt
  jmp .loop
