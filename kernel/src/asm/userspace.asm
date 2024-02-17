global as_call_userspace
global as_enable_syscall

section .text
; rdi: run
; rsi: stack
; rdx: rflags
as_call_userspace:
  mov ax, 0x1B
  mov ds, ax
  mov es, ax
	mov fs, ax
	mov gs, ax

  push rax
  add ax, 0x08

  push rsi
  push rdx
  push rax
  push rdi
  ; APIC EOI
  o64 iret

; rdi: syscall_handler
as_enable_syscall:
  ; Write syscall_handler into LSTAR
  mov eax, edi
  shr rdi, 32
  mov edx, edi
  mov rcx, 0xc0000082 ; IA32_LSTAR
  wrmsr

  ; Write segment bases in STAR
  mov rcx, 0xC0000081
  rdmsr
  mov edx, 0x00130008
  wrmsr

  ; Enable syscall through EFER
  mov rcx, 0xc0000080
	rdmsr
	or eax, 1
	wrmsr

  ret