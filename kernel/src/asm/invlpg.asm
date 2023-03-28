global as_invlpg

section .text

as_invlpg:
  invlpg [rdi]
  ret
