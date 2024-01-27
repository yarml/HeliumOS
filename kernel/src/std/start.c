#include <acpi.h>
#include <apic.h>
#include <boot_info.h>
#include <cfgtb.h>
#include <dev.h>
#include <interrupts.h>
#include <kterm.h>
#include <mem.h>
#include <mutex.h>
#include <pci.h>
#include <proc.h>
#include <stdio.h>
#include <sys.h>

#include <asm/msr.h>

void __init_stdlib();
int  kmain();

void _start() {
  {
    // stop all secondary cores
    // they should wait to be started by Helium
    if (!proc_isprimary()) {
      proc_ignition_wait();
    }
  }
  int_disable();

  printd("BSPID: %u\n", bootboot.bspid);

  printd("Disabling legacy PIC\n");
  pic_disable();

  printd("Initializing memory.\n");
  mem_init();

  printd("Initializing stdlib.\n");
  __init_stdlib();

  printd("Initializing interrupts.\n");
  int_init();

  printd("Initializing filesystem.\n");
  fs_init();

  printd("Initializing config tables\n");
  cfgtb_init();

  printd("Initializing kernel terminal\n");
  kterm_init();

  printd("Loading kernel modules\n");

  printd("PCI Probe\n");
  pci_probe();

  printd("ACPI Lookup\n");
  acpi_lookup();

  dev_init();

  proc_ignite();
}