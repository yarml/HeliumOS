#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

#define SYSCALL_EXIT (0x10)
#define SYSCALL_PRINT (0x11)

uint64_t dosyscall(uint64_t syscall, ...);

void syscall_print(char *str);
void syscall_exit(uint64_t status);

#endif