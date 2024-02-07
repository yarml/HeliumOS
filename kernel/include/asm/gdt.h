#ifndef HELIUM_ASM_GDT_H
#define HELIUM_ASM_GDT_H

#include <stdint.h>

#include <arch/mem.h>

void as_setup_gdt(
    gdtr *reg, uint16_t data_sel, uint16_t code_sel, uint16_t tss_sel
);

#endif