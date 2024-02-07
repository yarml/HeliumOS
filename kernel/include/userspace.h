#ifndef HELIUM_USERSPACE_H
#define HELIUM_USERSPACE_H

#include <stdint.h>

#define USERSPACE_EXEC_VPTR (void *)((uintptr_t)2 * 1024 * 1024 * 1024)
#define USERSPACE_MAINEXEC_SIZE ((size_t)1022 * 1024 * 1024 * 1024)

#define USPACE_STACK_SIZE (2 * 1024 * 1024)
#define USPACE_STACK_BASE ((void *)((size_t)17 * 1024 * 1024 * 1024 * 1024))
#define USPACE_STACK_TOP ((void *)(USPACE_STACK_BASE - USPACE_STACK_SIZE))

int  exec();
void syscall(uint64_t rdi);

#endif