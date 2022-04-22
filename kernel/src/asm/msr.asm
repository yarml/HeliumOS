global as_ldefer

%include "asm/msr.h.asm"

as_ldefer:
    mov ecx, MSR_IA32_EFER
    rdmsr
    ret