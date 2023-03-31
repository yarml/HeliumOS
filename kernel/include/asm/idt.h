#ifndef HELIUM_ASM_IDT_H
#define HELIUM_ASM_IDT_H

#include <interrupts.h>

void as_lidt(idt *idtr);
void as_sidt(idt *idtr);

#endif