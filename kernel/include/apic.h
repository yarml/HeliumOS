#ifndef HELIUM_APIC_H
#define HELIUM_APIC_H

#include <attributes.h>
#include <mem.h>
#include <stdint.h>

typedef struct APIC_REGMAP apic_regmap;

struct APIC_REGMAP {
  uint32_t res0[4];

  uint32_t res1[4];

  uint32_t idreg[4];
  uint32_t verreg[4];

  uint32_t res2[4][4];

  uint32_t taskprreg[4];  // TPR
  uint32_t arbprreg[4];
  uint32_t procprreg[4];
  uint32_t eoireg[4];
  uint32_t remoterreg[4];
  uint32_t logdestreg[4];  // LDR
  uint32_t destfmtreg[4];  // DFR
  uint32_t sivreg[4];

  uint32_t isr[8][4];
  uint32_t tmr[8][4];
  uint32_t irr[8][4];

  uint32_t errstatreg[4];  // ESR

  uint32_t res3[6][4];

  uint32_t lvt_cmcireg[4];

  uint32_t icr[2][4];

  uint32_t lvt_timerreg[4];
  uint32_t lvt_thrmlsnsreg[4];
  uint32_t lvt_perfmonreg[4];
  uint32_t lvt_lint0reg[4];
  uint32_t lvt_lint1reg[4];
  uint32_t lvt_errreg[4];

  uint32_t initcountreg[4];
  uint32_t currcountreg[4];

  uint32_t res4[4][4];

  uint32_t divcfgreg[4];  // DCR

  uint32_t res5[193][4];
} pack;

#define APIC_BASE ((void *)0xFEE00000)

void     apic_init();
uint32_t apic_getid();

#endif