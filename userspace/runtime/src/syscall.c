#include <syscall.h>

void syscall_exit(uint64_t status) {
  dosyscall(SYSCALL_EXIT, status);
}

void syscall_print(char const *str) {
  dosyscall(SYSCALL_PRINT, str);
}

void syscall_dbg(char const *str) {
  dosyscall(SYSCALL_DBG, str);
}

size_t syscall_gets(char *buf, size_t len) {
  return dosyscall(SYSCALL_GETS, buf, len);
}

void syscall_clear() {
  dosyscall(SYSCALL_CLEAR);
}
void syscall_kcfg(uint16_t r, uint16_t g, uint16_t b) {
  dosyscall(SYSCALL_KCFG, r, g, b);
}
void syscall_kcbg(uint16_t r, uint16_t g, uint16_t b) {
  dosyscall(SYSCALL_KCBG, r, g, b);
}
