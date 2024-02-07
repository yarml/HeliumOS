#ifndef HELIUM_ASM_USERSPACE_H
#define HELIUM_ASM_USERSPACE_H

#include <attributes.h>
#include <stddef.h>

noreturn void as_call_userspace(void *run, void *stack, size_t rflags);
void          as_enable_syscall(void *syscall_handler);

#endif