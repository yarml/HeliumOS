#include <interrupts.h>
#include <boot_info.h>
#include <initrd.h>
#include <stdlib.h>
#include <cpuid.h>
#include <stdio.h>
#include <ctype.h>
#include <utils.h>
#include <mem.h>
#include <sys.h>

#include <fs/tar.h>

void __init_stdio();
void __init_stdlib();
int kmain();

// Initialize C stdlib then call kmain()
void _start()
{
  {
    // stop all secondary cores
    // they should wait to be started by Helium
    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);
    if(b >> 24 != bootboot.bspid)
    {
      halt();
      // If we ever leave this halt, then stdio was prooooobably intialized
      printf("[Core %d] Unhalted... Stopping.\n", b >> 24);

    }
  }

  // Disable interrupts for now
  int_disable();

  // We can't printf("Initializing stdio") :P
  __init_stdio();
  printf("Initialized stdio.\n");


  printf("Initializing memory structures.\n");
  mem_init();

  printf("Initializing stdlib.\n");
  __init_stdlib();

  printf("Initializing interrupts.\n");
  int_init(); // This will also enable interrupts

  // Map initrd into virtual memory
  printf("Initializing filesystem.\n");
  fs_init();

  printf("Calling main function.\n");
  kmain();

  printf("stop()\n");
  stop();
}