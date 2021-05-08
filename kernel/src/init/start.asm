%include "debug/debug.asm"


global _start
global magic
global mbd
global destroy_objects

extern setup_gdt


extern kmain
extern init_memory
extern dbg_out

extern start_ctors
extern end_ctors
extern start_dtors
extern end_dtors
 
MODULEALIGN equ 1<<0
MEMINFO     equ 1<<1

FLAGS       equ  (MODULEALIGN | MEMINFO)
MAGIC       equ   0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)
 

section multiboot

align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
 
STACKSIZE equ 0x800000

align 4096
basic_pt_1:
%assign i 0
%rep 1024
    dd (i << 12) | 0x007
%assign i i+1
%endrep
basic_pt_2:
%assign i 1024
%rep 1024
    dd (i << 12) | 0x007
%assign i i+1
%endrep
basic_pt_3:
%assign i 2048
%rep 1024
    dd (i << 12) | 0x007
%assign i i+1
%endrep
basic_pt_4:
%assign i 3072
%rep 1024
    dd (i << 12) | 0x007
%assign i i+1
%endrep


align 4096
basic_page_dir:
%rep 1024
    dd 0
%endrep

basic_gdt:
    dq 0x0000000000000000
    dq 0x00CF9B000000FFFF
    dq 0x00CF93000000FFFF
basic_gdtr:
    dw 0x18
    dd basic_gdt - 0xC0000000

_start:
    cli
    mov  esp, stack_bottom - 0xC0000000 + STACKSIZE
    mov  [magic - 0xC0000000], eax
    mov  [mbd - 0xC0000000], ebx
; Setup a basic gdt
    lgdt [basic_gdtr - 0xC0000000]
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    jmp  0x08:(far_jump - 0xC0000000)
far_jump:
; Setup a basic paging
    mov eax, basic_pt_1 - 0xC0000000
    or eax, 0x007
    mov dword [basic_page_dir - 0xC0000000 + 4 * 0  ], eax
    mov dword [basic_page_dir - 0xC0000000 + 4 * 768], eax

    mov eax, basic_pt_2 - 0xC0000000
    or eax, 0x007
    mov dword [basic_page_dir - 0xC0000000 + 4 * 1  ], eax
    mov dword [basic_page_dir - 0xC0000000 + 4 * 769], eax

    mov eax, basic_pt_3 - 0xC0000000
    or eax, 0x007
    mov dword [basic_page_dir - 0xC0000000 + 4 * 2  ], eax
    mov dword [basic_page_dir - 0xC0000000 + 4 * 770], eax

    mov eax, basic_pt_4 - 0xC0000000
    or eax, 0x007
    mov dword [basic_page_dir - 0xC0000000 + 4 * 3  ], eax
    mov dword [basic_page_dir - 0xC0000000 + 4 * 771], eax

    ; Enable paging
    mov eax, basic_page_dir - 0xC0000000
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    add esp, 0xC0000000

    mov eax, higher_half
    jmp eax
higher_half:
    
    call setup_gdt
    push dword [mbd]
    call init_memory
    add esp, 4
    mov ebx, start_ctors
    jmp .ctors_until_end
.call_constructor:
    call [ebx]
    add  ebx,4
.ctors_until_end:
    cmp  ebx, end_ctors
    jb   .call_constructor

    call kmain
    
.hang:
    jmp .hang

destroy_objects:
    mov  ebx, end_dtors
    jmp  .dtors_until_end
.call_destructor:
    sub  ebx, 4
    call [ebx]
.dtors_until_end:
    cmp  ebx, start_dtors
    ja   .call_destructor

    ret

section .bss
 
align 4
magic: resd 1
mbd:   resd 1
stack_bottom:
    resb STACKSIZE
stack_top:

section .rodata
start_msg:
    db 0xA, "Called _start", 0xA, 0x0
