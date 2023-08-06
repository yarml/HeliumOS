#include <apic.h>
#include <mutex.h>
#include <proc.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <sys.h>

static atomic_int ignition  = 0;
static mutex      init_lock = 0;

void proc_waitinit() {
  while (!ignition) {
    pause();
  }
  proc_init();
}

void proc_ignite() {
  ignition = 1;
  proc_waitinit();
}

void proc_init() {
  uint32_t procid = apic_getid();

  mutex_lock(&init_lock);
  printd("[Proc %u] Initialization\n", procid);
  mutex_ulock(&init_lock);
  stop();
}
