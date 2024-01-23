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
    APIC_VBASE->divcfgreg[0]    = TIMER_DIVCFG(0b111);
    APIC_VBASE->initcountreg[0] = bus_freq * 1000;
  }

  mutex_ulock(&apic_init_lock);
}

uint32_t apic_getid() {
  uint32_t a, b, c, d;
  __cpuid(1, a, b, c, d);

  return b >> 24;
}

void apic_acpi_entry_handler(acpi_header *head) {
  madt              *table                = (void *)head;
  size_t             parsed_len           = sizeof(madt);
  madt_entry_header *current_entry_header = table->first;

  printf("LAPIC_ADR: %p, FLAGS: %x\n", table->lapic_adr, table->flags);

  while (parsed_len < table->header.len) {
    parsed_len += current_entry_header->len;

    switch (current_entry_header->type) {
      case MADT_LAPIC: {
        madt_lapic *lapic = (void *)current_entry_header;
        printf(
            "\tLAPIC: ACPI Proc ID: %u, APIC ID: %u, FLAGS: %x\n",
            lapic->acpi_procid,
            lapic->apic_id,
            lapic->flags
        );
      } break;
      case MADT_IOAPIC: {
        madt_ioapic *ioapic = (void *)current_entry_header;
        printf(
            "\tIOAPIC: IO APIC ID: %u, IO APIC Adr: %x, GSIB: "
            "%x\n",
            ioapic->ioapic_id,
            ioapic->ioapic_adr,
            ioapic->gsib
        );
      } break;
      case MADT_IOAPIC_INTER_SRC_OVERRIDE: {
        madt_ioapic_iso *ioapic_iso = (void *)current_entry_header;
        printf(
            "\tIOAPIC ISO: Bus source: %x, IRQ source: %x, GSI: %x, Flags: "
            "%x\n",
            ioapic_iso->bus_source,
            ioapic_iso->irq_source,
            ioapic_iso->gsi,
            ioapic_iso->flags
        );
      } break;
      default:
        printf("\tUnknown MADT entry type: %x\n", current_entry_header->type);
        break;
    }
    current_entry_header =
        (void *)current_entry_header + current_entry_header->len;
  }
}
