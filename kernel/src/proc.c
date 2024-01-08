#include <apic.h>
#include <boot_info.h>
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

uint32_t proc_getid() { return apic_getid(); }

int proc_isprimary() { return apic_getid() == bootboot.bspid; }

void proc_ignition_wait() {
  int_disable();
  proc_ignition_wait_done(PROC_IGNITION_GDT);
  // Once BSP sets up the GDT, we load it too
  load_gdt();

  proc_ignition_wait_done(PROC_IGNITION_DONE);
  proc_init();
}

void proc_ignite() {
  ignition = 1;
  proc_ignition_wait();
}

void proc_init() {
  mutex_lock(&init_lock);
  printd("[Proc %&] Initialization\n");

  int_load_and_enable();

  mutex_ulock(&init_lock);

  if (proc_isprimary()) {
    kmain();
  }

  printd("[Proc %&] Stop");
  stop();
}

void proc_ignition_mark_done(int ignition_process) {
  ignition |= ignition_process;
}

void proc_ignition_wait_done(int ignition_process) {
  while (!(ignition & ignition_process)) {
    pause();
  }
}
