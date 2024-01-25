#ifndef HELIUM_APIC_H
#define HELIUM_APIC_H

#include <acpi.h>
#include <attributes.h>
#include <mem.h>
#include <stdint.h>

typedef union APIC_MSR {
  struct {
    uint64_t res0      : 8;
    uint64_t bsp       : 1;
    uint64_t res1      : 2;
    uint64_t enable    : 1;
    uint64_t apic_base : 52;
  } pack;
  uint64_t reg;
} apic_msr;

#define APIC_MSR_APIC_BASE(apic_msr) (apic_msr.reg & 0xFFFFFFFFFFFFF000)

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

typedef union LVT_ERROR {
  struct {
    uint32_t vector : 8;
    uint32_t res0   : 8;
    uint32_t mask   : 1;
    uint32_t res1   : 15;
  } pack;
  uint32_t reg;
} pack lvt_error;

typedef union LVT_TIMER {
  struct {
    uint32_t vector       : 8;
    uint32_t res0         : 4;
    uint32_t deliv_status : 1;
    uint32_t res1         : 3;
    uint32_t mask         : 1;
    uint32_t mode         : 2;
    uint32_t res2         : 13;
  } pack;
  uint32_t reg;
} pack lvt_timer;

typedef union SIV_REG {
  struct {
    uint32_t vector         : 8;
    uint32_t apic_enable    : 1;
    uint32_t foc_proc_check : 1;
    uint32_t res0           : 2;
    uint32_t eoi_supress    : 1;
    uint32_t res1           : 19;
  } pack;
  uint32_t reg;
} pack siv_reg;

typedef union IOREDTBL {
  uint64_t reg;
  struct {
    uint32_t low;
    uint32_t high;
  } pack;
  struct {
    uint64_t vector     : 8;
    uint64_t deliv_mode : 3;
    uint64_t dest_mode  : 1;
    uint64_t deliv_stat : 1;
    uint64_t pinpol     : 1;
    uint64_t remote_irr : 1;
    uint64_t trig_mode  : 1;
    uint64_t mask       : 1;
    uint64_t res0       : 39;
    uint64_t dest       : 8;

  } pack;
} pack ioredtbl;

#define APIC_BASE ((void *)0xFEE00000)
#define APIC_VBASE                                                             \
  ((apic_regmap *)(KVMSPACE + (uint64_t)1024 * 1024 * 1024 * 1024 +            \
                   (uint64_t)512 * 1024 * 1024 * 1024))

typedef volatile struct IOAPIC_REGMAP {
  uint32_t regsel;
  uint32_t res0[3];
  uint32_t regwin;
} pack ioapic_regmap;

typedef struct IOAPIC_INFO {
  ioapic_regmap *regmap;
  size_t         id;
  size_t         irq_base;
  size_t         len;
} ioapic_info;

#define IOAPIC_VBASE                                                           \
  (KVMSPACE + (uint64_t)1024 * 1024 * 1024 * 1024 +                            \
   (uint64_t)513 * 1024 * 1024 * 1024)

#define TIMER_DIVCFG(mode) ((mode & 0b11) | ((mode & 0b100) << 1))

void     apic_init();
uint32_t apic_getid();
void     apic_eoi();

size_t ioapic_find_redirection(size_t irq);
void   ioapic_set_handler(size_t irq, size_t vector);

void apic_acpi_entry_handler(acpi_header *table);

// I don't want to create a new header file for this guy
void pic_disable();

#endif