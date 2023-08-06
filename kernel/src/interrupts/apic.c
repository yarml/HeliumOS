#include <apic.h>
#include <cpuid.h>
#include <mem.h>
#include <stdio.h>
#include <vcache.h>

#include <asm/msr.h>

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

  uint32_t apic_id  = vptr->idreg[0];
  uint32_t apic_ver = vptr->verreg[0];

  printd("APIC ID: %08x\n", apic_id);
  printd("APIC VERSION: %08x\n", apic_ver);

  mem_vumap(vptr, 0x1000);
}
