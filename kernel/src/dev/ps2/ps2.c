#include "ps2.h"

#include <apic.h>

#include <asm/io.h>
#include <dev/ps2.h>

static void set_scancode_set(int set) {
  while (1) {
    as_outb(0x60, 0xF0);
    as_iowait();
    as_outb(0x60, set);
    as_iowait();

    uint8_t response = as_inb(0x60);
    if (response == 0xFA) {
      break;
    }
  }
}

static void config_typematic() {
  while (1) {
    as_outb(0x60, 0xFA);
    as_iowait();
    uint8_t response = as_inb(0x60);
    if (response == 0xFA) {
      break;
    }
  }
}

void ps2_init() {
  set_scancode_set(2);
  config_typematic();

  size_t irq = ioapic_find_redirection(1);
  ioapic_set_handler(irq, PS2_KBD_INTVEC);

  i_kbdstate = (kbd_state){0};
}

kbd_state i_kbdstate;
