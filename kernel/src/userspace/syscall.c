#include <mem.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>
#include <userspace.h>

void syscall(
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
      break;
    default:
      printd("Unknown system call: %lx\n", rdi);
      as_event_loop();
  }
}