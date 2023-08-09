#ifndef HELIUM_PROC_H
#define HELIUM_PROC_H

#include <stdint.h>

void proc_ignite();
void proc_ignition_wait();
void proc_init();

uint32_t proc_getid();
int      proc_isprimary();

#endif