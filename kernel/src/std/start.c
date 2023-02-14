#include <boot_info.h>
#include <cpuid.h>
#include <stdio.h>
#include <mem.h>
#include <sys.h>

void __init_stdio();

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
      stop();
  }
  // We can't printf("Initializing stdio") :P
  __init_stdio();
  printf("Initialized stdio\n");

  printf("Initializing memory structures\n");
  mem_init();

  printf("Calling main function\n");
  kmain();

  printf("halt()\n");
  halt();
}