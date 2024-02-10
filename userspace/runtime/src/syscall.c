#include <syscall.h>

void syscall_print(char *str) {
  dosyscall(SYSCALL_PRINT, str);
}

void syscall_exit(uint64_t status) {
  dosyscall(SYSCALL_EXIT, status);
}
