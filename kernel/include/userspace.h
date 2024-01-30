#ifndef HELIUM_USERSPACE_H
#define HELIUM_USERSPACE_H

#include <stdint.h>

#define USERSPACE_EXEC_VPTR (void *)((uintptr_t)2 * 1024 * 1024 * 1024)
#define USERSPACE_MAINEXEC_SIZE ((size_t)1022 * 1024 * 1024 * 1024)

int exec();

#endif