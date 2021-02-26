%include "debug.asm"

global _start
global magic
global mbd

extern setup_gdt


extern kmain
 
extern start_ctors
extern end_ctors
extern start_dtors
extern end_dtors
 
MODULEALIGN equ  1<<0
MEMINFO     equ  1<<1

FLAGS       equ  (MODULEALIGN | MEMINFO)
MAGIC       equ   0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)
 

section multiboot

align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
 
STACKSIZE equ 0x4000
 
_start:
    cli    

    mov  esp, stack + STACKSIZE
    mov  [magic], eax
    mov  [mbd], ebx
 
    mov  ebx, start_ctors
    jmp  .ctors_until_end
.call_constructor:
    call [ebx]
    add  ebx,4
.ctors_until_end:
    cmp  ebx, end_ctors
    jb   .call_constructor

    call setup_gdt

    push dword [mbd]
    call kmain
    add esp, 4

    mov  ebx, end_dtors
    jmp  .dtors_until_end
.call_destructor:
    sub  ebx, 4
    call [ebx]
.dtors_until_end:
    cmp  ebx, start_dtors
    ja   .call_destructor
.hang:
    jmp .hang
 
section .bss
 
align 4
magic: resd 1
mbd:   resd 1
stack: resb STACKSIZE