#include <interrupts.h>
#include <boot_info.h>
#include <cpuid.h>
#include <stdio.h>
#include <mem.h>
#include <sys.h>

void __init_stdlib();
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
    {
      halt();
      printd("[Core %d] Unhalted... Stopping.\n", b >> 24);
      stop();
    }
  }
  // Draw debugging sqaures
  int x, y;
  int s = bootboot.fb_scanline;

  // Red square to signal Helium booted
  for(y=0;y<20;y++)
    for(x=0;x<20;x++)
      *((uint32_t*)(&fb + s*(y+20) + (x+20)*4))=0x00FF0000;

  // Disable interrupts for now
  int_disable();

  printd("Initializing memory structures.\n");
  mem_init();

  printd("Initializing stdlib.\n");
  __init_stdlib();

  printd("Initializing interrupts.\n");
  int_init(); // This will also enable interrupts

  // Map initrd into virtual memory
  printd("Initializing filesystem.\n");
  fs_init();

  __init_stdio();

  // Green square to signal that all systems prior to kmain got initialized
  for(y=0;y<20;y++)
    for(x=0;x<20;x++)
      *((uint32_t*)(&fb + s*(y+20) + (x+50)*4))=0x0000FF00;

  printd("Calling main function.\n");
  kmain();

  // Blue square to signal that everything went probably fine, or at least
  // we didn't crash
  for(y=0;y<20;y++)
    for(x=0;x<20;x++)
      *((uint32_t*)(&fb + s*(y+20) + (x+80)*4))=0x000000FF;

  fprintf(stdout, "Test?");

  printd("stop()\n");
  stop();
}