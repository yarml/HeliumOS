#ifndef HELIUM_USERSPACE_H
#define HELIUM_USERSPACE_H

#include <stdint.h>

#define USPACE_STACK_SIZE (2 * 1024 * 1024)
#define USPACE_STACK_BASE ((void *)((size_t)48 * 1024 * 1024 * 1024 * 1024))
#define USPACE_STACK_TOP ((void *)(USPACE_STACK_BASE - USPACE_STACK_SIZE))

void exec();
uint64_t syscall(
    uint64_t rdi,
    uint64_t rsi,
    uint64_t rdx,
    uint64_t rcx,
    uint8_t  r8,
    uint64_t r9
);

#define SYSCALL_EXIT (0x10)
#define SYSCALL_PRINT (0x11)
#define SYSCALL_GETS (0x12)
#define SYSCALL_DBG (0x13)
#define SYSCALL_CLEAR (0x14)
#define SYSCALL_KCFG (0x15)
#define SYSCALL_KCBG (0x16)

#endif