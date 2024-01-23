#include <apic.h>
#include <boot_info.h>
#include <cpuid.h>
#include <interrupts.h>
#include <mutex.h>
#include <proc.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>

int kmain();

// Bitmap of which ignition processes have been completed by BSP
static atomic_int ignition  = 0;
static mutex      init_lock = 0;

uint32_t proc_getid() {
  return apic_getid();
}

int proc_isprimary() {
  return apic_getid() == bootboot.bspid;
}

void proc_ignition_wait() {
  int_disable();
  proc_ignition_wait_step(PROC_IGNITION_GDT);
  // Once BSP sets up the GDT, we load it too
  load_gdt();

  proc_ignition_wait_step(PROC_IGNITION_DONE);
  proc_init();
}

void proc_ignite() {
  proc_ignition_mark_step(PROC_IGNITION_DONE);
  proc_init();
}

void proc_init() {
  mutex_lock(&init_lock);

  int_load_and_enable();

  apic_init();

  mutex_ulock(&init_lock);

  if (proc_isprimary()) {
    kmain();
    int a = 0;
    while (1) {
      printf("\r%d", a);
      a++;
      halt();
    }
  }
  while (1) {
    halt();
  }
}

void proc_ignition_mark_step(int ignition_step) {
  ignition = ignition | (1 << ignition_step);
}

void proc_ignition_wait_step(int ignition_step) {
  while (!(ignition & (1 << ignition_step))) {
    pause();
  }
}

uint32_t proc_bus_freq() {
  uint32_t bus_freq;

  uint32_t a, b, c, d;
  // Check if we're in a VM
  __cpuid(1, a, b, c, d);
  if (c & (1 << 31)) {
    // If running in a VM, will just set CPU frequency to 1000MHz, accuracy
    // doesn't matter In this context.
    bus_freq = 100;
  } else {
    __cpuid(0x16, a, b, c, d);
    bus_freq = c;
  }

  return bus_freq;
}
