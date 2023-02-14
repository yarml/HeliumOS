global as_outb

as_outb:
  mov rdx, rdi
  mov rax, rsi
  out dx, al
  ret