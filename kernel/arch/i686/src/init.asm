; Constants
MBALIGN   equ  1 << 0
MEMINFO   equ  1 << 1
FLAGS     equ  MBALIGN | MEMINFO
MAGIC     equ  0x1BADB002
CHECKSUM  equ -(MAGIC + FLAGS)
STACKSIZE equ 16*1024

; externs
extern load_arch

;---------------------------------;

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .bss
align 16
stack:
    resb STACKSIZE
.top:

section .text
global _start:function (_start.end - _start)
_start:
	mov esp, stack.top
 
	call load_arch

	cli
.hang:	
	hlt
	jmp .hang
.end:
