#include <acpi.h>
#include <boot_info.h>
#include <cfgtb.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

#include <dts/hashtable.h>

static void *map_table(
    dts_hashtable *acpi_vmemmap, void *padr, size_t *pgindex
) {
  void *al_padr = (void *)((uintptr_t)padr & 0xFFFFFFFFFFFFF000);
  void *vadr    = ACPI_TABLE_VBASE + *pgindex * 0x1000;

  // Not C++'s VTable

  mem_vmap(vadr, al_padr, 0x1000, 0);

  acpi_header *vtable = vadr + (uintptr_t)padr % 0x1000;
  size_t       alsize = ALIGN_UP(vtable->len, 0x1000);

  *pgindex += 1;

  if (alsize > 0x1000) {
    size_t rest = alsize - 0x1000;
    mem_vmap(vadr + 0x1000, al_padr + 0x1000, rest, 0);
    *pgindex += rest / 0x1000;
  }

  dts_hashtable_insert(acpi_vmemmap, al_padr, vadr);

  return vtable;
}

static void *get_vadr(
    dts_hashtable *acpi_vmemmap, void *padr, size_t *pgindex
) {
  void  *al_padr = (void *)((uintptr_t)padr & 0xFFFFFFFFFFFFF000);
  size_t mod     = (uintptr_t)padr % 0x1000;
  bool   found   = false;

  void *res = dts_hashtable_search(acpi_vmemmap, al_padr, &found);
  if (found) {
    return res + mod;
  }

  return map_table(acpi_vmemmap, padr, pgindex);
}

static void walk_acpi_recursive(
    acpi_header *head, dts_hashtable *acpi_vmemmap, size_t *pgindex
) {
  // Verify checksum
  uint8_t checksum = memsum(head, head->len);
  if (checksum) {
    printd("Invalid cheksum(%u)\n", checksum);
    return;
  }
  // XSDT always handled by us
  if (!memcmp(head, "XSDT", 4)) {
    xsdt  *table    = (void *)head;
    size_t nentries = (table->header.len - sizeof(xsdt)) / 8;
    printd("\n");
    for (size_t i = 0; i < nentries; ++i) {
      // Find if this physical page is already mapped
      acpi_header *next_table =
          get_vadr(acpi_vmemmap, (void *)table->ss_list[i], pgindex);
      walk_acpi_recursive(next_table, acpi_vmemmap, pgindex);
    }
  } else {
    if (!cfgtb_acpi_callhandlers((char *)head, head)) {
      printf("No ACPI handler for %.4s\n", head);
    }
  }
}

void acpi_lookup() {
  dts_hashtable *acpi_vmemmap = dts_hashtable_create_uptrkey(0);
  size_t         pgindex      = 0;

  acpi_header *xsdt =
      get_vadr(acpi_vmemmap, (void *)bootboot.arch.x86_64.acpi_ptr, &pgindex);

  walk_acpi_recursive(xsdt, acpi_vmemmap, &pgindex);
}
