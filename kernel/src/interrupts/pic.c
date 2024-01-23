// The only reason we talk with the PIC is to disable the mfer.

#include <asm/io.h>

void pic_disable() {
  as_outb(0x21, 0xFF);
  as_outb(0xA1, 0xFF);
}