global as_outb
global as_outsb

as_outb:
  mov rdx, rdi
  mov rax, rsi
  out dx, al
  ret

as_outsb:
  cld
  mov rcx, rdx
  mov rdx, rdi
  rep outsb
  ret