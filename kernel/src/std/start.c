#include <boot_info.h>
#include <cpuid.h>
#include <interrupts.h>
#include <mem.h>
#include <mutex.h>
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

    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);
    uint32_t id = b >> 24;

    uint64_t apic_base = as_smsr(MSR_IA32_APIC_BASE);

    uint8_t bsp           = apic_base & 0x100;
    uint8_t global_enable = apic_base & 0x800;

    uint32_t base = apic_base & 0xFFFFFF000;

    printd(
        "[Core %u] BSP: %u, G ENABLE: %u, BASE: %08x\n",
        id,
        bsp,
        global_enable,
        base
    );  // This printd is wild when there are multiple cores
    mutex_ulock(&proc_lock);
    if (id != bootboot.bspid) {
      halt();
      printd("[Core %u] Unhalted... Stopping.\n", id);
      stop();
    }
  }

  while (proc_count != bootboot.numcores) { }

  printd("BSPID: %u\n", bootboot.bspid);

  // Disable interrupts for now
  int_disable();

  printd("Initializing memory structures.\n");
  mem_init();

  printd("Initializing stdlib.\n");
  __init_stdlib();

  printd("Initializing interrupts.\n");
  int_init();  // This will also enable interrupts

  // Map initrd into virtual memory
  printd("Initializing filesystem.\n");
  fs_init();

  printd("Calling main function.\n");
  kmain();

  printf("Proc count: %u\n", proc_count);

  printd("stop()\n");
  stop();
}