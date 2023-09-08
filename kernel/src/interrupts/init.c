#include <error.h>
#include <interrupts.h>
#include <mem.h>
#include <mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>

#include <asm/idt.h>

#include "handlers.h"

// Interrupt handler

// A cleaner structure to define all the IDT entries, which gets converted
// to the processor's structure
typedef struct IDT_ENTRY_INFO idt_entry_info;

struct IDT_ENTRY_INFO {
  void    *handler;
  uint16_t seg_sel;
  uint16_t dpl;
  uint16_t type;
};

static idt_entry_info kernel_idt_image[256] = {
    [0] =
        {.handler = exception_div,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [8] =
        {.handler = exception_double_fault,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [12] =
        {.handler = exception_stackseg_fault,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [13] =
        {.handler = exception_general_prot,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [14] =
        {.handler = exception_page_fault,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
};

static idt_entry kernel_idt[256];
static mutex     idt_lock = 0;

static idt_entry decode_entry_info(idt_entry_info info) {
  return (idt_entry){
      .offset0 = (uintptr_t)info.handler & 0xFFFF,
      .offset1 = (uintptr_t)info.handler >> 16,
      .seg_sel = info.seg_sel,
      .ist     = 0,
      .dpl     = 0,
      .type    = IDT_TYPE_INT,
      .present = 1,
  };
}

void int_init() {
  // No need to lock the interrupt table because this function is only
  // executed at startup

  memset(kernel_idt, 0, sizeof(kernel_idt));

  for (size_t i = 0; i < 256; ++i) {
    idt_entry_info *info = kernel_idt_image + i;

    if (!info->handler) {
      continue;
    }

    kernel_idt[i] = decode_entry_info(*info);
  }
}

void int_load_and_enable() {
  mutex_lock(&idt_lock);
  idt idtr;
  idtr.limit  = sizeof(kernel_idt) - 1;
  idtr.offset = kernel_idt;

  as_lidt(&idtr);
  int_enable();
  mutex_ulock(&idt_lock);
}
