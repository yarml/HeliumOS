#ifndef HELIUM_CFGTB_H
#define HELIUM_CFGTB_H

#include <dts/hashtable.h>

typedef void (*cfgtb_acpi_handler)();

void cfgtb_init();
void cfgtb_acpi_register(char *entry_sig, cfgtb_acpi_handler handler);
void cfgtb_acpi_callhandlers(char *entry_sig);

#endif