#ifndef HELIUM_ASM_STOS_H
#define HELIUM_ASM_STOS_H

#include <stdint.h>

extern void as_stosb(uint64_t block, uint64_t c, uint64_t size);
extern void as_stosw(uint64_t block, uint64_t c, uint64_t size);
extern void as_stosd(uint64_t block, uint64_t c, uint64_t size);
extern void as_stosq(uint64_t block, uint64_t c, uint64_t size);

#endif