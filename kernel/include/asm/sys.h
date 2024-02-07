#ifndef HELIUM_ASM_SYS_H
#define HELIUM_ASM_SYS_H

#include <attributes.h>

noreturn void as_sys_stop();
noreturn void as_event_loop();

// No call
void as_syscall_handle();

#endif