global as_outb
global as_outsb

global as_outw
global as_outsw

global as_outd
global as_outsd

section .text

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

as_outw:
  mov rdx, rdi
  mov rax, rsi
  out dx, ax
  ret

as_outsw:
  cld
  mov rcx, rdx
  mov rdx, rdi
  rep outsw
  ret

as_outd:
  mov rdx, rdi
  mov rax, rsi
  out dx, eax
  ret

as_outsd:
  cld
  mov rcx, rdx
  mov rdx, rdi
  rep outsd
  ret

