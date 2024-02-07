#include <mem.h>
#include <proc.h>
#include <string.h>

#include <asm/gdt.h>

void gdt_proc_setup(proc_info *info) {
  memset(info->gdt, 0, sizeof(info->gdt));

  // GDT 1 is kernel code segment
  info->gdt[1].nsys    = 1;
  info->gdt[1].exec    = 1;
  info->gdt[1].dpl     = 0;
  info->gdt[1].lmode   = 1;
  info->gdt[1].present = 1;

  // GDT 2 is kernel data segment
  info->gdt[2].nsys    = 1;
  info->gdt[2].write   = 1;
  info->gdt[2].dpl     = 0;
  info->gdt[2].present = 1;

  // GDT 3 is user data segment
  info->gdt[3].nsys    = 1;
  info->gdt[3].write   = 1;
  info->gdt[3].dpl     = 0;
  info->gdt[3].present = 1;

  // GDT 4 is user code segment
  info->gdt[4].nsys    = 1;
  info->gdt[4].exec    = 1;
  info->gdt[4].dpl     = 0;
  info->gdt[4].lmode   = 1;
  info->gdt[4].present = 1;

  tss   *tssp      = &(info->proc_tss);
  size_t tss_limit = sizeof(info->proc_tss) - 1;

  tss_entry *tsse = (void *)(info->gdt + 5);
  tsse->nsys      = 0;
  tsse->type      = 0b1001;
  tsse->base0     = (uintptr_t)tssp & 0xFFFFFF;
  tsse->base1     = ((uintptr_t)tssp >> 24) & 0xFFFFFFFFFF;
  tsse->limit0    = tss_limit & 0xFFFF;
  tsse->limit1    = (tss_limit >> 16) & 0xF;
  tsse->dpl       = 0;
  tsse->present   = 1;

  tssp->rsp[0] = (uintptr_t)info->ksatck;
  tssp->ist[0] = (uintptr_t)info->nmi_stack;  // IST1 used by NMI
  tssp->ist[1] = (uintptr_t)info->df_stack;   // IST 2 used by DF

  load_gdt(
      info->gdt,
      sizeof(info->gdt),
      MEM_KERNEL_DATA_DESC,
      MEM_KERNEL_CODE_DESC,
      MEM_TSS_DESC
  );
}

void load_gdt(
    gdt_entry *gdt,
    size_t     size,
    uint16_t   data_sel,
    uint16_t   code_sel,
    uint16_t   tss_sel
) {
  gdtr reg;

  reg.limit = size - 1;
  reg.base  = gdt;

  as_setup_gdt(&reg, data_sel, code_sel, tss_sel);
}
