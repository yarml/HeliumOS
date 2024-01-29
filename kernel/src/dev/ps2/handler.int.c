#include <apic.h>
#include <interrupts.h>
#include <kterm.h>
#include <stdio.h>

#include <asm/io.h>
#include <dev/ps2.h>

#include "ps2.h"

interrupt_handler void ps2_kbd_int(int_frame *frame) {
  int_disable();
  apic_eoi();
  uint8_t scancode = as_inb(PS2_PORT_DATA);
  int     press    = !(scancode & 0x80);
  scancode &= 0x7F;

  // For keys that can be pressed multiple times
  int delta = press ? 1 : -1;

  if (scancode == KEY_CTL) {
    i_kbdstate.ctrl += delta;
    return;
  }
  if (i_kbdstate.ctrl) {  // we don't handle control yet
    return;
  }

  switch (scancode) {
    case KEY_SHIFT:
      i_kbdstate.shift += delta;
      break;
    case KEY_ALT:
      i_kbdstate.alt += delta;
      break;
    case KEY_CAPS:
      i_kbdstate.caps += delta;
      break;
    default: {
      if (!press) {
        break;
      }
      // Yes there is a better way to do this
      bool  shift  = !!i_kbdstate.caps ^ !!i_kbdstate.shift;
      char *layout = i_kbdstate.alt ? kbd_dn_alt
                   : shift          ? kbd_dn_shifted
                                    : kbd_dn;
      char  c      = layout[scancode];
      if (c) {
        kterm_putcin(c);
      }
    } break;
  }
}
