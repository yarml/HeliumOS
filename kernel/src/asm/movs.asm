global as_movsq
global as_movsd
global as_movsw
global as_movsb

global as_rmovsq
global as_rmovsd
global as_rmovsw
global as_rmovsb

section .text

; The only parameters we will need to move are rdx to rcx for the count,
; otherwise, rdi and rsi are what they should be
as_movsq:
  cld
  mov rcx, rdx
  rep movsq
  ret

as_movsd:
  cld
  mov rcx, rdx
  rep movsd
  ret

as_movsw:
  cld
  mov rcx, rdx
  rep movsw
  ret

as_movsb:
  cld
  mov rcx, rdx
  rep movsb
  ret

; reverse copy
as_rmovsq:
  std
  mov rcx, rdx
  rep movsq
  ret

as_rmovsd:
  std
  mov rcx, rdx
  rep movsd
  ret

as_rmovsw:
  std
  mov rcx, rdx
  rep movsw
  ret

as_rmovsb:
  std
  mov rcx, rdx
  rep movsb
  ret
