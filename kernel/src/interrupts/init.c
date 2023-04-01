#include <interrupts.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <mem.h>
#include <sys.h>

#include <asm/idt.h>

#include "handlers.h"

// Interrupt handler

// A cleaner structure to define all the IDT entries, which gets converted
// to the processor's structure
typedef struct IDT_ENTRY_INFO idt_entry_info;

struct IDT_ENTRY_INFO
{
  void *handler;
  uint16_t seg_sel;
  uint16_t dpl;
  uint16_t type;
};

static idt_entry_info kernel_idt_image[256] = {
  [0] = {
    .handler = exception_div,
    .seg_sel = MEM_KERNEL_CODE_DESC,
    .dpl = 0,
    .type = IDT_TYPE_INT
  },
  [14] = {
    .handler = exception_page_fault,
    .seg_sel = MEM_KERNEL_CODE_DESC,
    .dpl = 0,
    .type = IDT_TYPE_INT
  }
};

static idt_entry kernel_idt[256];

void int_init()
{
  memset(kernel_idt, 0, sizeof(kernel_idt));

  for(size_t i = 0; i < 256; ++i)
  {
    idt_entry_info *info = kernel_idt_image + i;

    if(!info->handler)
      continue;

    kernel_idt[i].offset0 = (uintptr_t) info->handler & 0xFFFF;
    kernel_idt[i].offset1 = (uintptr_t) info->handler >> 16;

    kernel_idt[i].seg_sel = info->seg_sel;

    kernel_idt[i].ist = 0;
    kernel_idt[i].dpl = 0;
    kernel_idt[i].type = IDT_TYPE_INT;

    kernel_idt[i].present = 1;
  }

  idt idtr;
  idtr.limit = sizeof(kernel_idt) - 1;
  idtr.offset = kernel_idt;

  as_lidt(&idtr);

  int_enable();
}
