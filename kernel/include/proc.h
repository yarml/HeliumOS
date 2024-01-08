#ifndef HELIUM_PROC_H
#define HELIUM_PROC_H 0


#include <stdint.h>

// Bitmap indices for ignition
#define PROC_IGNITION_GDT 0
#define PROC_IGNITION_DONE 31



void proc_ignite();
void proc_ignition_wait();
void proc_init();

uint32_t proc_getid();
int      proc_isprimary();

void proc_ignition_mark_done(int ignition_process);
void proc_ignition_wait_done(int ignition_process);

#endif