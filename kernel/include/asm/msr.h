#ifndef HELIUM_ASM_MSR_H
#define HELIUM_ASM_MSR_H

#include <stdint.h>

#define MSR_IA32_EFER (0xC0000080)
#define MSR_IA32_APIC_BASE (0x1B)

uint64_t as_smsr(uint32_t regn);
void     as_lmsr(uint32_t regn, uint64_t val);

#endif