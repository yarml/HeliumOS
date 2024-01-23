#include <apic.h>
#include <cfgtb.h>
#include <string.h>

#include <dts/stack.h>

static dts_hashtable *acpi_handlers;

void cfgtb_init() {
  acpi_handlers = dts_hashtable_create_uptrkey(0);

  cfgtb_acpi_register("APIC", apic_acpi_entry_handler);
}

void cfgtb_acpi_register(char *entry_sig, cfgtb_acpi_handler handler) {
  // entry_sig is a 4byte string, not null terminated
  // we convert it to a u64 with the high 4 bytes set to 0
  uintptr_t sig = 0;
  memcpy(&sig, entry_sig, 4);

  dts_stack *handlers_stack =
      dts_hashtable_search(acpi_handlers, (void *)sig, 0);
  if (!handlers_stack) {
    handlers_stack = dts_stack_create(sizeof(uintptr_t), 0);
    dts_hashtable_insert(acpi_handlers, (void *)sig, handlers_stack);
  }

  dts_stack_push(handlers_stack, &handler);
}

size_t cfgtb_acpi_callhandlers(char *entry_sig, acpi_header *table) {
  uintptr_t sig = 0;
  memcpy(&sig, entry_sig, 4);

  dts_stack *handlers_stack =
      dts_hashtable_search(acpi_handlers, (void *)sig, 0);
  if (!handlers_stack) {
    return 0;
  }

  for (size_t i = 0; i < handlers_stack->len; ++i) {
    cfgtb_acpi_handler handler =
        *(cfgtb_acpi_handler *)dts_stack_at(handlers_stack, i, 0);
    handler(table);
  }
  return handlers_stack->len;
}