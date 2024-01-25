#include "ps2.h"

#include <apic.h>
#include <interrupts.h>
#include <kmod.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>

#include <asm/io.h>

#include "interrupt.h"


static void post() {
  size_t int_num;
  int_register(ps2_int, &int_num);

  // Check if IRQ 1 is redirected
  size_t irq = ioapic_find_redirection(1);
  printd("IRQ1->%lu\n", irq);
  // Then redirect IRQ1 to the interrupt we just setup
  ioapic_set_handler(irq, int_num);
}

kmod_ft module_init() {
  // We need to do our setup in POST
  return (kmod_ft){.post = post};
}
