#ifndef RT_SYSCALL_H
#define RT_SYSCALL_H

#include <stddef.h>
#include <stdint.h>

#define SYSCALL_EXIT (0x10)
#define SYSCALL_PRINT (0x11)
#define SYSCALL_GETS (0x12)
#define SYSCALL_DBG (0x13)
#define SYSCALL_CLEAR (0x14)
#define SYSCALL_KCFG (0x15)
#define SYSCALL_KCBG (0x16)

uint64_t dosyscall(uint64_t syscall, ...);

void syscall_exit(uint64_t status);
void syscall_print(char const *str);
void syscall_dbg(char const *str);
size_t syscall_gets(char *buf, size_t len);
void syscall_clear();
void syscall_kcfg(uint16_t r, uint16_t g, uint16_t b);
void syscall_kcbg(uint16_t r, uint16_t g, uint16_t b);


#endif