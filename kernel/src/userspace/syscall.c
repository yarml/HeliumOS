#include <stdint.h>
#include <stdio.h>
#include <sys.h>

void syscall(uint64_t rdi) {
  printd("SYSCALL RDI: %lu\n", rdi);
  stop();
}