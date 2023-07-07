#ifndef HELIUM_ASM_CTLR_H
#define HELIUM_ASM_CTLR_H

#include <stdint.h>

#include <arch/ctlr.h>

ctlr_cr3_npcid as_rcr3();
ctlr_cr3_npcid as_rlcr3();

uint64_t       as_rcr2();

#endif