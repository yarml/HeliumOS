global as_smsr
global as_lmsr

as_smsr:
  mov ecx, edi
  rdmsr
  shl rdx, 32
  or rax, rdx
  ret

as_lmsr:
  mov ecx, edi ; set ecx to regn
  mov eax, esi ; set eax to lower bits of val
  shr rsi, 32  ; shift to lower the higher bits of val
  mov rdx, rsi ; set edx to higher bits of val
  wrmsr
  ret