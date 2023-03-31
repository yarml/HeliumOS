global as_lidt
global as_sidt

section .text

as_lidt:
  lidt [rdi]
  ret

as_sidt:
  sidt [rdi]
  ret
