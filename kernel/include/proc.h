#ifndef HELIUM_PROC_H
#define HELIUM_PROC_H 0

#include <stddef.h>
#include <stdint.h>

#include <arch/mem.h>

#define KSTACK_SIZE (16 * 1024)
#define NMI_STACK_SIZE (4 * 1024)
#define DF_STACK_SIZE (4 * 1024)

#define PROC_TABLE_VPTR (KVMSPACE + (uintptr_t)2 * 1024 * 1024 * 1024 * 1024)
#define PROC_TABLE_VSIZE ((uintptr_t)2 * 1024 * 1024 * 1024 * 1024)

// One of these is created per CPU
typedef struct PROC_INFO {
  size_t sysid;  // Assigned by Helium
  size_t apicid;
  gdt_entry
      gdt[7];  // Entry 0: NULL, Entry 1: Kernel code, Entry 2: Kernel data,
               // Entry 3: user data, Entry 4: user code, Entry 5&6: TSS
  tss   proc_tss;
  void *ksatck;
  void *nmi_stack;  // The enemmy stack
  void *df_stack;
} proc_info;

void proc_ignite();
void proc_ignition_wait();
void proc_init();

uint32_t proc_getid();
int      proc_isprimary();

uint32_t proc_bus_freq();

void       proc_register(uint32_t apic_id, proc_info *info);
size_t     proc_numcores();
proc_info *proc_getinfo();

#endif