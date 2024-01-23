#include <apic.h>
#include <cpuid.h>
#include <interrupts.h>
#include <mem.h>
#include <mutex.h>
#include <proc.h>
#include <stdio.h>
#include <vcache.h>

#include <asm/ctlr.h>
#include <asm/msr.h>

mutex apic_init_lock;

// This function must be executed by all cores at the same time
// AP cores will wait for BSP to map a region of memory before they actually
// initialze their local APIC.
void apic_init() {
  apic_msr apic_msr  = {.reg = as_smsr(MSR_IA32_APIC_BASE)};
  uint64_t apic_base = APIC_MSR_APIC_BASE(apic_msr);

  if (apic_base != (uintptr_t)APIC_BASE) {
    apic_msr.apic_base = (uintptr_t)APIC_BASE >> 12;
    as_lmsr(MSR_IA32_APIC_BASE, apic_msr.reg);
  }

  if (!proc_isprimary()) {
    proc_ignition_wait_step(PROC_IGNITION_APIC_MAP);
    as_rlcr3();
  } else {
    mem_vmap(APIC_VBASE, APIC_BASE, 0x1000, 0);
    proc_ignition_mark_step(PROC_IGNITION_APIC_MAP);
  }
  mutex_lock(&apic_init_lock);

  printd("[Proc %&] APIC init\n");
  printd("\tAPIC version: %02x\n", APIC_VBASE->verreg[0] & 0xFF);

  // Setup a timer for the BSP
  if (proc_isprimary()) {
    lvt_error err = {.reg = 0};
    err.vector    = 0xFD;

    APIC_VBASE->lvt_errreg[0] = err.reg;

    siv_reg siv     = {.reg = 0};
    siv.apic_enable = 1;
    siv.vector      = 0xFF;

    APIC_VBASE->sivreg[0] = siv.reg;

    printd("TPR: %x\n", APIC_VBASE->taskprreg[0]);

    uint32_t bus_freq = proc_bus_freq();

    lvt_timer timer = {.reg = 0};
    timer.vector    = 0xFE;
    timer.mode      = 0b01;

    APIC_VBASE->lvt_timerreg[0] = timer.reg;
    APIC_VBASE->divcfgreg[0]    = TIMER_DIVCFG(0b010);
    APIC_VBASE->initcountreg[0] = bus_freq * 10000000;
  }

  mutex_ulock(&apic_init_lock);
}

void apic_init2() {
  uint32_t a, b, c, d;
  __cpuid(1, a, b, c, d);

  uint32_t apic_support = d & 0x200;

  if (!apic_support) {
    error_feature("APIC");
  }

  mutex_lock(&apic_init_lock);

  uint64_t apic_msr = as_smsr(MSR_IA32_APIC_BASE);

  uint8_t bsp           = (apic_msr & 0x100) >> 8;
  uint8_t global_enable = (apic_msr & 0x800) >> 11;

  uint64_t apic_base = apic_msr & 0xFFFFFFFFFFFFF000;

  printd("[Proc %&] APIC BASE: %016x\n", apic_base);

  printd("BSP: %u\n", bsp);
  printd("G ENABLE: %u\n", global_enable);
  printd("BASE: %08x\n", apic_base);

  apic_regmap *vptr = KVMSPACE + (uint64_t)1024 * 1024 * 1024 * 1024 +
                      (uint64_t)512 * 1024 * 1024 * 1024;
  printd("VPTR: %p\n", vptr);
  mem_vmap(vptr, (void *)(uintptr_t)apic_base, 0x1000, 0);

  uint32_t apic_id     = vptr->idreg[0];
  uint32_t apic_verinf = vptr->verreg[0];

  uint8_t ver               = apic_verinf & 0xFF;
  uint8_t maxlvt_count      = (apic_verinf & 0xFF0000) >> 16;
  uint8_t broadcast_supress = (apic_verinf & 0x1000000) >> 24;

  printd("APIC ID: %08x\n", apic_id);
  printd("APIC VERSION: %02x\n", ver);
  printd("MAX LVT COUNT: %02x\n", maxlvt_count);
  printd("ABILITY[SUPRESS EOI BROADCAST]: %03b\n", broadcast_supress);

  uint32_t lvt_timer_reg = vptr->lvt_timerreg[0];
  // lvt_timer lvttimer          = decode_lvt_timer_reg(lvt_timer_reg);
  // uint32_t  recoded_timer_reg = encode_lvt_timer_reg(lvttimer);

  printd("LVT Timer: %08x\n", lvt_timer_reg);
  // printd("Recoded: %08x\n", recoded_timer_reg);

  mem_vumap(vptr, 0x1000);

  mutex_ulock(&apic_init_lock);
  return;
}

uint32_t apic_getid() {
  uint32_t a, b, c, d;
  __cpuid(1, a, b, c, d);

  return b >> 24;
}
