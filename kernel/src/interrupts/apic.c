#include <apic.h>
#include <cpuid.h>
#include <mem.h>
#include <stdio.h>
#include <vcache.h>

#include <asm/msr.h>

typedef struct LVT_TIMER {
  uint64_t vector       : 8;
  uint64_t res0         : 4;
  uint64_t deliv_status : 1;
  uint64_t res1         : 3;
  uint64_t mask         : 1;
  uint64_t mode         : 2;
  uint64_t res2         : 13;
} lvt_timer;

static lvt_timer decode_lvt_timer_reg(uint32_t reg) {
  return (lvt_timer){
      .vector       = reg & 0xFF,
      .deliv_status = (reg >> 12) & 1,
      .mask         = (reg >> 16) & 0b01,
      .mode         = (reg >> 17) & 0b11,
      .res0         = 0,
      .res1         = 0,
      .res2         = 0,
  };
}

static uint32_t encode_lvt_timer_reg(lvt_timer reg) {
  return reg.vector | reg.deliv_status << 12 | reg.mask << 16 | reg.mode << 17;
}

void apic_init() {
  uint32_t a, b, c, d;
  __cpuid(1, a, b, c, d);

  uint32_t apic_support = d & 0x200;

  if (!apic_support) {
    error_feature("APIC");
  }

  printd("Initializing APIC\n");
  uint64_t apic_base = as_smsr(MSR_IA32_APIC_BASE);

  uint8_t bsp           = (apic_base & 0x100) >> 8;
  uint8_t global_enable = (apic_base & 0x800) >> 11;

  uint32_t base = apic_base & 0xFFFFFF000;

  printd("APIC BASE: %016x\n", apic_base);

  printd("BSP: %u\n", bsp);
  printd("G ENABLE: %u\n", global_enable);
  printd("BASE: %08x\n", base);

  apic_regmap *vptr = KVMSPACE + (uint64_t)1024 * 1024 * 1024 * 1024 +
                      (uint64_t)513 * 1024 * 1024 * 1024;
  printd("VPTR: %p\n", vptr);
  mem_vmap(vptr, (void *)(uintptr_t)base, 0x1000, 0);

  uint32_t apic_id     = vptr->idreg[0];
  uint32_t apic_verinf = vptr->verreg[0];

  uint8_t ver               = apic_verinf & 0xFF;
  uint8_t maxlvt_count      = (apic_verinf & 0xFF0000) >> 16;
  uint8_t broadcast_supress = (apic_verinf & 0x1000000) >> 24;

  printd("APIC ID: %08x\n", apic_id);
  printd("APIC VERSION: %02x\n", ver);
  printd("MAX LVT COUNT: %02x\n", maxlvt_count);
  printd("ABILITY[SUPRESS EOI BROADCAST]: %b\n", broadcast_supress);

  uint32_t  lvt_timer_reg     = vptr->lvt_timerreg[0];
  lvt_timer lvttimer          = decode_lvt_timer_reg(lvt_timer_reg);
  uint32_t  recoded_timer_reg = encode_lvt_timer_reg(lvttimer);

  printd("LVT Timer: %08x\n", lvt_timer_reg);
  printd("Recoded: %08x\n", recoded_timer_reg);

  mem_vumap(vptr, 0x1000);
}

uint32_t apic_getid() {
  uint32_t a, b, c, d;
  __cpuid(1, a, b, c, d);

  return b >> 24;
}
