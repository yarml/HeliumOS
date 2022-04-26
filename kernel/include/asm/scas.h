#ifndef HELIUM_ASM_SCAS_H
#define HELIUM_ASM_SCAS_H

#include <stdint.h>

uint64_t as_scasb (uint64_t block, uint64_t c, uint64_t count);
uint64_t as_scasw (uint64_t block, uint64_t c, uint64_t count);
uint64_t as_scasd (uint64_t block, uint64_t c, uint64_t count);
uint64_t as_scasq (uint64_t block, uint64_t c, uint64_t count);
uint64_t as_nscasb(uint64_t block, uint64_t c, uint64_t count);
uint64_t as_nscasw(uint64_t block, uint64_t c, uint64_t count);
uint64_t as_nscasd(uint64_t block, uint64_t c, uint64_t count);
uint64_t as_nscasq(uint64_t block, uint64_t c, uint64_t count);

#endif