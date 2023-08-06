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

static _Atomic uint8_t proc_count = 0;
static mutex           proc_lock  = 0;

// Initialize C stdlib then call kmain()
void _start() {
  {
    // stop all secondary cores
    // they should wait to be started by Helium

    mutex_lock(&proc_lock);
    ++proc_count;

    uint32_t id = apic_getid();

    uint32_t apic_base = as_smsr(MSR_IA32_APIC_BASE);

    uint8_t bsp           = (apic_base & 0x100) >> 8;
    uint8_t global_enable = (apic_base & 0x800) >> 11;

    uint32_t base = apic_base & 0xFFFFFF000;

    printd(
        "[Proc %u] BSP: %u, G ENABLE: %u, BASE: %08x",
        id,
        bsp,
        global_enable,
        base
    );
    if (!bsp) {
      printd(" waiting for initialization...\n");
      mutex_ulock(&proc_lock);
      proc_waitinit();
    } else {
      printd(" will perform initialization...\n");
      mutex_ulock(&proc_lock);
    }
  }

  while (proc_count != bootboot.numcores) {
    pause();
  }

  printd("BSPID: %u\n", bootboot.bspid);

  // Disable interrupts for now
  int_disable();

  printd("Initializing memory structures.\n");
  mem_init();

  printd("Initializing stdlib.\n");
  __init_stdlib();

  printd("Initializing interrupts.\n");
  int_init();  // This will also enable interrupts

  apic_init();

  // Map initrd into virtual memory
  printd("Initializing filesystem.\n");
  fs_init();

  // Load kernel modules
  kmod_loadall();

  // Use proper files for stdio
  __init_stdio();

  // printd("Calling main function.\n");
  // kmain();

  proc_ignite();
}