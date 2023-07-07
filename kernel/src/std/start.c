#include <boot_info.h>
#include <cpuid.h>
#include <interrupts.h>
#include <mem.h>
#include <stdio.h>
#include <sys.h>

void __init_stdlib();
void __init_stdio();
int  kmain();

// Initialize C stdlib then call kmain()
void _start() {
  {
    // stop all secondary cores
    // they should wait to be started by Helium
    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);
    if (b >> 24 != bootboot.bspid) {
      halt();
      printd("[Core %d] Unhalted... Stopping.\n", b >> 24);
      stop();
    }
  }

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

  printd("stop()\n");
  stop();
}