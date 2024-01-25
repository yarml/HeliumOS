#include <apic.h>
#include <cpuid.h>
#include <interrupts.h>
#include <mem.h>
#include <mutex.h>
#include <proc.h>
#include <stdio.h>
#include <stdlib.h>
#include <vcache.h>

#include <asm/ctlr.h>
#include <asm/msr.h>
#include <dts/hashtable.h>
#include <dts/stack.h>

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

void apic_eoi() {
  APIC_VBASE->eoireg[0] = 0;
}

void ioapic_write(ioapic_regmap *apic_base, uint8_t offset, uint32_t val) {
  apic_base->regsel = offset;
  apic_base->regwin = val;
}
uint32_t ioapic_read(ioapic_regmap *apic_base, uint8_t offset) {
  apic_base->regsel = offset;
  return apic_base->regwin;
}

static dts_stack     *ioapic_stack             = 0;
static dts_hashtable *ioapic_redirection_table = 0;

static void ioapic_register(uint32_t id, void *phyadr, uint32_t int_base) {
  static size_t  pgindex = 0;
  ioapic_regmap *vbase   = IOAPIC_VBASE + pgindex * 0x1000;
  ++pgindex;
  mem_vmap((void *)vbase, phyadr, 0x1000, 0);

  uint32_t maxredent = ((ioapic_read(vbase, 1) >> 16) & 0b11111111);

  ioapic_info *info = calloc(1, sizeof(ioapic_info));
  info->id          = id;
  info->irq_base    = int_base;
  info->len         = maxredent + 1;
  info->regmap      = vbase;

  if (!ioapic_stack) {
    ioapic_stack = dts_stack_create(sizeof(void *), 0);
  }

  dts_stack_push(ioapic_stack, &info);
}

static void ioapic_redirection(size_t irq_src, size_t irq) {
  if (!ioapic_redirection_table) {
    ioapic_redirection_table = dts_hashtable_create_uptrkey(0);
  }

  dts_hashtable_insert(ioapic_redirection_table, (void *)irq_src, (void *)irq);
}

size_t ioapic_find_redirection(size_t irq) {
  bool found = false;

  // clang-format off
  // fsr clang-format screws this line up
  size_t redirected_irq = (size_t)
    dts_hashtable_search(
        ioapic_redirection_table, (void *)irq, &found
    );
  // clang-format on

  if (!found) {
    return irq;
  }
  return redirected_irq;
}

void ioapic_set_handler(size_t irq, size_t vector) {
  // Find which IOAPIC handles this IRQ
  ioapic_info *target = 0;
  for (size_t i = 0; i < ioapic_stack->len; ++i) {
    ioapic_info *info = *(ioapic_info **)dts_stack_at(ioapic_stack, i, 0);
    if (info->irq_base <= irq && irq < info->irq_base + info->len) {
      target = info;
      break;
    }
  }
  if (!target) {
    printd("NOTTT FOUND\n");
    return;
  }

  printd("FOUND\n");

  size_t regoff = 0x10 + (irq - target->irq_base) * 2;

  ioredtbl irqred = {.reg = 0};
  irqred.vector   = vector;

  ioapic_write(target->regmap, regoff, irqred.low);
  ioapic_write(target->regmap, regoff + 1, irqred.high);
}

void apic_acpi_entry_handler(acpi_header *head) {
  madt              *table                = (void *)head;
  size_t             parsed_len           = sizeof(madt);
  madt_entry_header *current_entry_header = table->first;

  while (parsed_len < table->header.len) {
    parsed_len += current_entry_header->len;

    switch (current_entry_header->type) {
      case MADT_LAPIC: {
        madt_lapic *lapic = (void *)current_entry_header;
        printd(
            "\tLAPIC: ACPI Proc ID: %u, APIC ID: %u, FLAGS: %x\n",
            lapic->acpi_procid,
            lapic->apic_id,
            lapic->flags
        );
      } break;
      case MADT_IOAPIC: {
        madt_ioapic *ioapic = (void *)current_entry_header;
        ioapic_register(
            ioapic->ioapic_id,
            (void *)(uintptr_t)ioapic->ioapic_adr,
            ioapic->gsib
        );
      } break;
      case MADT_IOAPIC_INTER_SRC_OVERRIDE: {
        madt_ioapic_iso *ioapic_iso = (void *)current_entry_header;
        if (ioapic_iso->bus_source) {
          // It's called being lazy
          printd("Unsupported IO/APIC bus redirection\n");
          break;
        }
        ioapic_redirection(ioapic_iso->irq_source, ioapic_iso->gsi);
      } break;
      default:
        printd("\tUnknown MADT entry type: %x\n", current_entry_header->type);
        break;
    }
    current_entry_header =
        (void *)current_entry_header + current_entry_header->len;
  }
}
