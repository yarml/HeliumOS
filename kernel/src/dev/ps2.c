#include <apic.h>

#include <dev/ps2.h>

void ps2_init() {
  size_t irq = ioapic_find_redirection(1);
  ioapic_set_handler(irq, PS2_KBD_INTVEC);
}