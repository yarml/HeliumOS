global as_outb
global as_outsb

global as_outw
global as_outsw

global as_outd
global as_outsd

global as_inb
global as_insb

global as_inw
global as_insw

global as_ind
global as_insd

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

as_inb:
  mov rdx, rdi
  in al, dx
  ret

as_insb:
  cld
  mov rcx, rdx
  mov rdx, rdi
  rep insb
  ret

as_inw:
  mov rdx, rdi
  in ax, dx
  ret

as_insw:
  cld
  mov rcx, rdx
  mov rdx, rdi
  rep insw
  ret

as_ind:
  mov rdx, rdi
  in eax, dx
  ret

as_insd:
  cld
  mov rcx, rdx
  mov rdx, rdi
  rep insd
  ret
