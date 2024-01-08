#include <apic.h>
#include <boot_info.h>
#include <cpuid.h>
#include <interrupts.h>
#include <kmod.h>
#include <mem.h>
#include <mutex.h>
#include <proc.h>
#include <stdio.h>
#include <sys.h>

#include <asm/msr.h>

void __init_stdlib();
void __init_stdio();
int  kmain();

void _start() {
  {
    // stop all secondary cores
    // they should wait to be started by Helium
    if (!proc_isprimary()) {
      proc_ignition_wait();
    }
  }

  printd("BSPID: %u\n", bootboot.bspid);

  int_disable();

  printd("Initializing memory.\n");
  mem_init();

  printd("Initializing stdlib.\n");
  __init_stdlib();

  printd("Initializing interrupts.\n");
  int_init();
  int_load_and_enable();

  printd("Initializing filesystem.\n");
  fs_init();

  printd("Loading kernel modules\n");
  kmod_loadall();

  printd("Initializing stdio\n");
  __init_stdio();

  printd("Calling main function.\n");
  kmain();

  proc_ignite();
}