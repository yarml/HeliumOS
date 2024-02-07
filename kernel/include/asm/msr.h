#ifndef HELIUM_ASM_MSR_H
#define HELIUM_ASM_MSR_H

#include <stdint.h>

#define MSR_IA32_APIC_BASE (0x1B)

#define MSR_IA32_EFER (0xC0000080)
#define MSR_IA32_STAR (0xC0000081)
#define MSR_IA32_LSTAR (0xC0000082)
#define MSR_IA32_CSTAR (0xC0000083)
#define MSR_IA32_SFMASK (0xC0000084)

uint64_t as_smsr(uint32_t regn);
void     as_lmsr(uint32_t regn, uint64_t val);

#endif