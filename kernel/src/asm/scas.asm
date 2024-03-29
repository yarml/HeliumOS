global as_scasb
global as_scasw
global as_scasd
global as_scasq

global as_nscasb
global as_nscasw
global as_nscasd
global as_nscasq

section .text

as_scasb:
  cld
  mov rax, rsi
  mov rcx, rdx
  repne scasb
  mov rax, rcx
  ret

as_scasw:
  cld
  mov rax, rsi
  mov rcx, rdx
  repne scasw
  mov rax, rcx
  ret

as_scasd:
  cld
  mov rax, rsi
  mov rcx, rdx
  repne scasd
  mov rax, rcx
  ret

as_scasq:
  cld
  mov rax, rsi
  mov rcx, rdx
  repne scasq
  mov rax, rcx
  ret


as_nscasb:
  cld
  mov rax, rsi
  mov rcx, rdx
  repe scasb
  mov rax, rcx
  ret

as_nscasw:
  cld
  mov rax, rsi
  mov rcx, rdx
  repe scasw
  mov rax, rcx
  ret

as_nscasd:
  cld
  mov rax, rsi
  mov rcx, rdx
  repe scasd
  mov rax, rcx
  ret

as_nscasq:
  cld
  mov rax, rsi
  mov rcx, rdx
  repe scasq
  mov rax, rcx
  ret
