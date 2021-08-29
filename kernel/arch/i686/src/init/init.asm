; Constants
MBALIGN   equ  1 << 0
MEMINFO   equ  1 << 1
FLAGS     equ  MBALIGN | MEMINFO
MAGIC     equ  0x1BADB002
CHECKSUM  equ -(MAGIC + FLAGS)
STACKSIZE equ 16*1024 ; TODO: Move this to config

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
	
	push eax
	push ebx
	
	call load_arch

	cli

	jmp i686_halt
.end:

global i686_halt:function
i686_halt:
	hlt
	jmp i686_halt


; Pseudo code
; section .multiboot:
; u32 MAGIC = $MAGIC
; u32 FLAGS = $FLAGS 
; u32 CHECKSUM = $CHECKSUM
; 
; section .bss:
; array<u8, $STACKSIZE> stack
;
; section .text:
; [[main]]
; [[global]]
;  _start(mbt_info = %ebx, mbt_sig = %eax)
; {
; 	%esp = stack.top() // Sets up stack
; 	load_arch(mbt_info, mbt_sig)
; 	i686_halt()
; }
; [[noreturn]]
; [[jump]]
; [[global]]
; i686_halt()
; {
; 	halt() // hlt instruction
;   i686_halt(); //
; }
