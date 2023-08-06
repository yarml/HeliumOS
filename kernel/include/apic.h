#ifndef HELIUM_APIC_H
#define HELIUM_APIC_H

#include <attributes.h>
#include <mem.h>
#include <stdint.h>

typedef struct APIC_REGMAP apic_regmap;

struct APIC_REGMAP {
  uint32_t res0_p0;
  uint32_t res0_p1;
  uint32_t res0_p2;
  uint32_t res0_p3;

  uint32_t res1_p0;
  uint32_t res1_p1;
  uint32_t res1_p2;
  uint32_t res1_p3;

  uint32_t idreg_p0;
  uint32_t idreg_p1;
  uint32_t idreg_p2;
  uint32_t idreg_p3;

  uint32_t verreg_p0;
  uint32_t verreg_p1;
  uint32_t verreg_p2;
  uint32_t verreg_p3;

  uint32_t res2_p0;
  uint32_t res2_p1;
  uint32_t res2_p2;
  uint32_t res2_p3;

  uint32_t res3_p0;
  uint32_t res3_p1;
  uint32_t res3_p2;
  uint32_t res3_p3;

  uint32_t res4_p0;
  uint32_t res4_p1;
  uint32_t res4_p2;
  uint32_t res4_p3;

  uint32_t res5_p0;
  uint32_t res5_p1;
  uint32_t res5_p2;
  uint32_t res5_p3;

  uint32_t taskprreg_p0;
  uint32_t taskprreg_p1;
  uint32_t taskprreg_p2;
  uint32_t taskprreg_p3;

  uint32_t arbprreg_p0;
  uint32_t arbprreg_p1;
  uint32_t arbprreg_p2;
  uint32_t arbprreg_p3;

  uint32_t procprreg_p0;
  uint32_t procprreg_p1;
  uint32_t procprreg_p2;
  uint32_t procprreg_p3;

  uint32_t eoireg_p0;
  uint32_t eoireg_p1;
  uint32_t eoireg_p2;
  uint32_t eoireg_p3;

  uint32_t remoterreg_p0;
  uint32_t remoterreg_p1;
  uint32_t remoterreg_p2;
  uint32_t remoterreg_p3;

  uint32_t logdestreg_p0;
  uint32_t logdestreg_p1;
  uint32_t logdestreg_p2;
  uint32_t logdestreg_p3;

  uint32_t destfmtreg_p0;
  uint32_t destfmtreg_p1;
  uint32_t destfmtreg_p2;
  uint32_t destfmtreg_p3;

  uint32_t sivreg_p0;
  uint32_t sivreg_p1;
  uint32_t sivreg_p2;
  uint32_t sivreg_p3;

  uint32_t isr0_p0;
  uint32_t isr0_p1;
  uint32_t isr0_p2;
  uint32_t isr0_p3;

  uint32_t isr1_p0;
  uint32_t isr1_p1;
  uint32_t isr1_p2;
  uint32_t isr1_p3;

  uint32_t isr2_p0;
  uint32_t isr2_p1;
  uint32_t isr2_p2;
  uint32_t isr2_p3;

  uint32_t isr3_p0;
  uint32_t isr3_p1;
  uint32_t isr3_p2;
  uint32_t isr3_p3;

  uint32_t isr4_p0;
  uint32_t isr4_p1;
  uint32_t isr4_p2;
  uint32_t isr4_p3;

  uint32_t isr5_p0;
  uint32_t isr5_p1;
  uint32_t isr5_p2;
  uint32_t isr5_p3;

  uint32_t isr6_p0;
  uint32_t isr6_p1;
  uint32_t isr6_p2;
  uint32_t isr6_p3;

  uint32_t isr7_p0;
  uint32_t isr7_p1;
  uint32_t isr7_p2;
  uint32_t isr7_p3;

  uint32_t tmr0_p0;
  uint32_t tmr0_p1;
  uint32_t tmr0_p2;
  uint32_t tmr0_p3;

  uint32_t tmr1_p0;
  uint32_t tmr1_p1;
  uint32_t tmr1_p2;
  uint32_t tmr1_p3;

  uint32_t tmr2_p0;
  uint32_t tmr2_p1;
  uint32_t tmr2_p2;
  uint32_t tmr2_p3;

  uint32_t tmr3_p0;
  uint32_t tmr3_p1;
  uint32_t tmr3_p2;
  uint32_t tmr3_p3;

  uint32_t tmr4_p0;
  uint32_t tmr4_p1;
  uint32_t tmr4_p2;
  uint32_t tmr4_p3;

  uint32_t tmr5_p0;
  uint32_t tmr5_p1;
  uint32_t tmr5_p2;
  uint32_t tmr5_p3;

  uint32_t tmr6_p0;
  uint32_t tmr6_p1;
  uint32_t tmr6_p2;
  uint32_t tmr6_p3;

  uint32_t tmr7_p0;
  uint32_t tmr7_p1;
  uint32_t tmr7_p2;
  uint32_t tmr7_p3;

  uint32_t irr0_p0;
  uint32_t irr0_p1;
  uint32_t irr0_p2;
  uint32_t irr0_p3;

  uint32_t irr1_p0;
  uint32_t irr1_p1;
  uint32_t irr1_p2;
  uint32_t irr1_p3;

  uint32_t irr2_p0;
  uint32_t irr2_p1;
  uint32_t irr2_p2;
  uint32_t irr2_p3;

  uint32_t rest[MEM_PS / 4 - 35 * 4];
} pack;

#define APIC_BASE ((void *)0xFEE00000)

void apic_init();

#endif