#ifndef HELIUM_ACPI_H
#define HELIUM_ACPI_H

#include <attributes.h>
#include <mem.h>
#include <stdint.h>

#define ACPI_TABLE_VBASE                                                       \
  (KVMSPACE + (uint64_t)1024 * 1024 * 1024 * 1024 +                            \
   (uint64_t)512 * 1024 * 1024 * 1024 + 4096)

typedef struct XSDP {
  char     signature[8];
  uint8_t  checksum;
  char     oem[6];
  uint8_t  rev;
  uint32_t rsdt_adr;

  uint32_t len;
  uint64_t xsdt_adr;
  uint8_t  xchecksum;
  uint8_t  res0[3];
} pack xsdp;

typedef struct ACPI_HEADER {
  char     sig[4];
  uint32_t len;
  uint8_t  rev;
  uint8_t  checksum;
  char     oem[6];
  char     oem_table_id[8];
  uint32_t oem_rev;
  uint32_t creator_id;
  uint32_t creator_rev;
} pack acpi_header;

typedef struct XSDT {
  acpi_header header;
  uint64_t    ss_list[];
} pack xsdt;

typedef struct MADT_ENTRY_HEADER {
  uint8_t type;
  uint8_t len;
} pack madt_entry_header;

typedef union MADT_APIC_FLAGS {
  uint32_t flags;
  struct {
    uint32_t res;
  } pack;
} pack madt_apic_flags;

typedef struct MADT {
  acpi_header       header;
  uint32_t          lapic_adr;
  uint32_t          flags;
  madt_entry_header first[];  // Only use to access the first element!
} pack madt;

typedef struct MADT_LAPIC {
  madt_entry_header header;
  uint8_t           acpi_procid;
  uint8_t           apic_id;
  union {
    uint32_t flags;
    struct {
      uint32_t proc_enabled : 1;
      uint32_t online_cap   : 1;
      uint32_t res0         : 30;
    } pack;
  } pack;
} pack madt_lapic;

typedef struct MADT_IOAPIC {
  madt_entry_header header;
  uint8_t           ioapic_id;
  uint8_t           res0;
  uint32_t          ioapic_adr;
  uint32_t          glob_sys_inter_base;
} pack madt_ioapic;

#define MADT_LAPIC (0)
#define MADT_IOAPIC (1)
#define MADT_IOAPIC_INTER_SRC_OVERRIDE (2)
#define MADT_IOAPIC_NMI_SRC (3)
#define MADT_LAPIC_NMI (4)
#define MADT_LAPIC_ADR_OVERRIDE (5)
#define MADT_LAPIC_x2APIC (9)

void acpi_lookup();

#endif