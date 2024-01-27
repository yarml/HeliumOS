#include <error.h>
#include <interrupts.h>
#include <mem.h>
#include <mutex.h>
#include <proc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>

#include <asm/idt.h>
#include <dev/ps2.h>

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
    [PS2_KBD_INTVEC] =
        {.handler = ps2_kbd_int,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [0xFD] =
        {.handler = apic_err,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [0xFE] =
        {.handler = timer_tick,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
    [0xFF] =
        {.handler = spurious_int,
             .seg_sel = MEM_KERNEL_CODE_DESC,
             .dpl     = 0,
             .type    = IDT_TYPE_INT},
};

static idt_entry_info default_idt_entry_info = {
    .handler = inter_unmapped,
    .seg_sel = MEM_KERNEL_CODE_DESC,
    .dpl     = 0,
    .type    = IDT_TYPE_INT};

static idt_entry kernel_idt[256];
static idt_entry bsp_idt[256];

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

  idt_entry default_idt_entry = decode_entry_info(default_idt_entry_info);

  for (size_t i = 0; i < 256; ++i) {
    idt_entry_info *info = kernel_idt_image + i;

    if (!info->handler) {
      kernel_idt[i] = default_idt_entry;
    } else {
      kernel_idt[i] = decode_entry_info(*info);
    }
  }

  memcpy(bsp_idt, kernel_idt, sizeof(kernel_idt));
}

void int_load_and_enable() {
  int_disable();
  idt idtr;

  idtr.limit  = sizeof(kernel_idt) - 1;
  idtr.offset = kernel_idt;

  as_lidt(&idtr);
  int_enable();
}
