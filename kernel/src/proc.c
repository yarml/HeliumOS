#include <apic.h>
#include <boot_info.h>
#include <cpuid.h>
#include <env.h>
#include <interrupts.h>
#include <kshell.h>
#include <mutex.h>
#include <proc.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>
#include <userspace.h>
#include <utils.h>

#include <asm/sys.h>
#include <asm/userspace.h>
#include <dts/hashtable.h>

// Set to 1 by BSP to tell other processors they can go to their event loop
static atomic_bool ignition  = 0;
static mutex       init_lock = 0;

// Hashtable indexable by APIC ID, there exist as many entries as CPUs in the
// system
static dts_hashtable *proc_table = 0;
static size_t         numcores   = 0;

uint32_t proc_getid() {
  return apic_getid();
}

int proc_isprimary() {
  return apic_getid() == bootboot.bspid;
}

void proc_ignition_wait() {
  while (!ignition) {
    pause();
  }
  proc_init();
}

void proc_ignite() {
  // A few things to do before waking up the other cores
  // Like allocating space for the stack table
  mem_vseg stacktableseg =
      mem_alloc_vblock(0x1000, MAPF_W | MAPF_R, STACK_TABLE_VPTR, 0x1000);

  if (stacktableseg.error) {
    error_out_of_memory("Could not allocate spa efor stack table");
  }
  // That's it for now

  ignition = true;
  proc_init();
}

void proc_init() {
  mutex_lock(&init_lock);

  // We need to figure out what is our stack
  int   x;
  void *ptr        = &x;
  void *stack_base = (void *)ALIGN_UP((uintptr_t)ptr, KSTACK_SIZE);

  proc_info *pinfo = proc_getinfo();

  // Stack base should be written in the stack table as well.
  STACK_TABLE_VPTR[pinfo->apicid] = stack_base;

  // Continue filling up proc_info of current processor
  pinfo->ksatck = stack_base;

  printd("[Proc %&] Stack base: %p\n", stack_base);
  printd("[Proc %&] NMI Stack base: %p\n", pinfo->nmi_stack);
  printd("[Proc %&] DF Stack base: %p\n", pinfo->df_stack);

  printd("Loading processor specific GDT\n");
  gdt_proc_setup(pinfo);

  int_load();
  apic_init();
  as_enable_syscall(as_syscall_handle);

  mutex_ulock(&init_lock);

  if (proc_isprimary()) {
    printd("Total number of cores: %lu\n", proc_numcores());
  }

  // Next, we go on a loop that simply halts repeatedly
  // I am doing it in assembly instead of C so that I can be 100% sure no
  // stack is being manipulated
  // This also enables interrupts
  as_event_loop();
}

uint32_t proc_bus_freq() {
  uint32_t bus_freq;

  bus_freq = env_busfreq();

  return bus_freq;
}

void proc_register(uint32_t apic_id, proc_info *info) {
  if (!proc_table) {
    proc_table = dts_hashtable_create_uptrkey(0);
    // Error handling my ass
  }

  void *apic_id_key = (void *)(uintptr_t)apic_id;

  bool found = false;
  dts_hashtable_search(proc_table, apic_id_key, &found);

  if (found) {  // should not happen
    printd("Found conflicting APIC IDs: %u\n", apic_id);
    return;
  }
  ++numcores;
  dts_hashtable_insert(proc_table, apic_id_key, info);
}

size_t proc_numcores() {
  return numcores;
}

proc_info *proc_getinfo() {
  return dts_hashtable_search(proc_table, (void *)(uintptr_t)proc_getid(), 0);
}
