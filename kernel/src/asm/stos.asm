global as_stosb
global as_stosw
global as_stosd
global as_stosq

as_stosb:
  cld
  mov rax, rsi
  mov rcx, rdx
  rep stosb
  ret

as_stosw:
  cld
  mov rax, rsi
  mov rcx, rdx
  rep stosw
  ret

as_stosd:
  cld
  mov rax, rsi
  mov rcx, rdx
  rep stosd
  ret

as_stosq:
  cld
  mov rax, rsi
  mov rcx, rdx
  rep stosq
  ret