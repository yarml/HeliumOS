#include <mem.h>
#include <stdint.h>
#include <stdio.h>
#include <kterm.h>
#include <sys.h>
#include <userspace.h>
#include <interrupts.h>

uint64_t syscall(
    uint64_t rdi,
    uint64_t rsi,
    uint64_t rdx,
    uint64_t rcx,
    uint8_t  r8,
    uint64_t r9
) {
  switch (rdi) {
    case SYSCALL_EXIT:
      printd("Exiting program with status code: %lu\n", rsi);
      as_event_loop();
    case SYSCALL_PRINT:
      if (rsi < (uintptr_t)KVMSPACE) {  // No printing kernel memory lol
        puts((char *)rsi);
      }
      return 0;
    case SYSCALL_DBG:
      if(rsi < (uintptr_t)KVMSPACE) {
        printd("%s", rsi);
      }
      return 0;
    case SYSCALL_GETS:
      if(rsi < (uintptr_t)KVMSPACE) {
        return kterm_read((void *)rsi, rdx);
      }
      return 0;
    case SYSCALL_CLEAR:
      kterm_clear();
      return 0;
    case SYSCALL_KCFG:
      kterm_setfg(rsi, rdx, rcx);
      return 0;
    case SYSCALL_KCBG:
      kterm_setbg(rsi, rdx, rcx);
      return 0;
    default:
      printd("Unknown system call: %lx\n", rdi);
      as_event_loop();
  }
}