#include <interrupts.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <sys.h>

#include <asm/idt.h>

#include "handlers.h"

static idt_entry kernel_idt[256];

void int_init()
{
  memset(kernel_idt, 0, sizeof(kernel_idt));


  kernel_idt[0].offset0 = (uintptr_t) div_err & 0xFFFF;
  kernel_idt[0].offset1 = (uintptr_t) div_err >> 16;

  kernel_idt[0].seg_sel = 0x8;
  kernel_idt[0].ist = 0;
  kernel_idt[0].type = IDT_TYPE_INT;
  kernel_idt[0].dpl = 0;
  kernel_idt[0].present = 1;

  idt idtr;
  idtr.limit = sizeof(kernel_idt) - 1;
  idtr.offset = kernel_idt;

  as_lidt(&idtr);

  int_enable();
}
