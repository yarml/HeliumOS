%include "debug/debug.asm"

global enable_paging
global reload_cr3

enable_paging:
; Go back to the lower half
    mov eax, lower_half - 0xC0000000
    jmp eax
lower_half:
; Disable paging
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
; Load new page directory
    mov eax, [esp + 4 - 0xC0000000]
    sub eax, 0xC0000000
    mov cr3, eax
; Enable Paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
; Back to the higher half
    mov eax, higher_half
    jmp eax
higher_half:
    ret

reload_cr3:
    mov eax, cr3
    mov cr3, eax
    ret