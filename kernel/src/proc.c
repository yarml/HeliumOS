#include <apic.h>
#include <boot_info.h>
#include <interrupts.h>
#include <mutex.h>
#include <proc.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>

static atomic_int ignition  = 0;
static mutex      init_lock = 0;

uint32_t proc_getid() { return apic_getid(); }

int proc_isprimary() { return apic_getid() == bootboot.bspid; }

void proc_ignition_wait() {
  int_disable();
  while (!ignition) {
    pause();
  }
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

  int a = 0;

  printd("[Proc %&] about to do an oopsie\n");

  int b = 10 / a;

  printd("%%%d%%\n", b);

  stop();
}
