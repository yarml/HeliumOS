global as_rcr3
global as_rlcr3

as_rcr3:
  mov rax, cr3
  ret

as_rlcr3:
  mov rax, cr3
  mov cr3, rax
  ret
