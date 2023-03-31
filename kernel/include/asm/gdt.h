#ifndef HELIUM_ASM_GDT_H
#define HELIUM_ASM_GDT_H

#include <stdint.h>

#include <arch/mem.h>

void as_lgdt(gdt *gdtr, uint16_t data_sel, uint16_t code_sel);
void as_sgdt(gdt *gdtr);

#endif