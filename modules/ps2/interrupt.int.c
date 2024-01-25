#include "interrupt.h"

#include <apic.h>
#include <stdio.h>

#include <asm/io.h>

#include "ps2.h"

interrupt_handler void ps2_int(int_frame *frame) {
  uint8_t scancode = as_inb(PS2_PORT_DATA);
  printd("Scancode: %02x\n", scancode);
  apic_eoi();
}