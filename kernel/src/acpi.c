#include <acpi.h>
#include <boot_info.h>
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

static void print_indent(size_t indent) {
  for (size_t i = 0; i < indent; ++i) {
    printd("--");
  }
  if (indent != 0) {
    printd(" ");
  }
}

static void print_acpi_recursive(
    size_t         indent,
    acpi_header   *head,
    dts_hashtable *acpi_vmemmap,
    size_t        *pgindex
) {
  print_indent(indent);
  printd("%.4s: ", head);
  indent++;

  if (head->len > 4096) {
    printd("Too large\n");
    return;
  }

  // Verify checksum
  uint8_t checksum = memsum(head, head->len);
  if (checksum) {
    printd("Invalid cheksum(%u)\n", checksum);
    return;
  }
  if (!memcmp(head, "XSDT", 4)) {
    xsdt  *table    = (void *)head;
    size_t nentries = (table->header.len - sizeof(xsdt)) / 8;
    printd("\n");
    for (size_t i = 0; i < nentries; ++i) {
      // Find if this physical page is already mapped
      acpi_header *next_table =
          get_vadr(acpi_vmemmap, (void *)table->ss_list[i], pgindex);
      print_acpi_recursive(indent, next_table, acpi_vmemmap, pgindex);
    }
  } else if (!memcmp(head, "APIC", 4)) {
    madt              *table                = (void *)head;
    size_t             parsed_len           = sizeof(madt);
    madt_entry_header *current_entry_header = table->first;

    printd("LAPIC_ADR: %p, FLAGS: %x", table->lapic_adr, table->flags);

    while (parsed_len < table->header.len) {
      printd("\n");
      print_indent(indent);
      parsed_len += current_entry_header->len;

      switch (current_entry_header->type) {
        case MADT_LAPIC: {
          madt_lapic *lapic = (void *)current_entry_header;
          printd(
              "LAPIC: ACPI Proc ID: %u, APIC ID: %u, FLAGS: %x",
              lapic->acpi_procid,
              lapic->apic_id,
              lapic->flags
          );
        } break;
        case MADT_IOAPIC: {
          madt_ioapic *ioapic = (void *)current_entry_header;
          printd(
              "IOAPIC: IO APIC ID: %u, IO APIC Adr: %x, Glob Sys Inter Base: "
              "%x",
              ioapic->ioapic_id,
              ioapic->ioapic_adr,
              ioapic->glob_sys_inter_base
          );
        } break;
        default:
          printd("Unknown MADT entry type: %x", current_entry_header->type);
          break;
      }
      current_entry_header =
          (void *)current_entry_header + current_entry_header->len;
    }
    printd("\n");
  } else {
    printd("Unknown\n");
  }
}

void acpi_lookup() {
  // TODO: Remove this when I'm confident in my ACPI abilities
  // acpi_parse();

  dts_hashtable *acpi_vmemmap = dts_hashtable_create_uptrkey(0);
  size_t         pgindex      = 0;

  acpi_header *xsdt =
      get_vadr(acpi_vmemmap, (void *)bootboot.arch.x86_64.acpi_ptr, &pgindex);

  printd("ACPI Lookup\n");
  print_acpi_recursive(0, xsdt, acpi_vmemmap, &pgindex);
}
