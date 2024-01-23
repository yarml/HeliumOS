#ifndef HELIUM_CFGTB_H
#define HELIUM_CFGTB_H

#include <acpi.h>

#include <dts/hashtable.h>

typedef void (*cfgtb_acpi_handler)(acpi_header *table);

void cfgtb_init();
void cfgtb_acpi_register(char *entry_sig, cfgtb_acpi_handler handler);
size_t cfgtb_acpi_callhandlers(char *entry_sig, acpi_header *table);

#endif