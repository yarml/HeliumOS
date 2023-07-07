#ifndef HELIUM_ASM_MOVS_H
#define HELIUM_ASM_MOVS_H

#include <stdint.h>

extern void as_movsq(uint64_t to, uint64_t from, uint64_t count);
extern void as_movsd(uint64_t to, uint64_t from, uint64_t count);
extern void as_movsw(uint64_t to, uint64_t from, uint64_t count);
extern void as_movsb(uint64_t to, uint64_t from, uint64_t count);

extern void as_rmovsq(uint64_t to, uint64_t from, uint64_t count);
extern void as_rmovsd(uint64_t to, uint64_t from, uint64_t count);
extern void as_rmovsw(uint64_t to, uint64_t from, uint64_t count);
extern void as_rmovsb(uint64_t to, uint64_t from, uint64_t count);

#endif