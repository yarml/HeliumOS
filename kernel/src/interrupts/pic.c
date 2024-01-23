// The only reason we talk with the PIC is to disable the mfer.

#include <asm/io.h>

void pic_disable() {
  as_outb(0x20, 0x11);
  as_iowait();
  as_outb(0xA0, 0x11);
  as_iowait();

  as_outb(0x21, 0x20);
  as_iowait();
  as_outb(0xA1, 0x20);
  as_iowait();
  as_outb(0x21, 4);
  as_iowait();
  as_outb(0xA1, 2);
  as_iowait();

  as_outb(0x21, 1);
  as_iowait();
  as_outb(0xA1, 1);
  as_iowait();

  as_outb(0x21, 0xFF);
  as_outb(0xA1, 0xFF);

  as_iowait();
}