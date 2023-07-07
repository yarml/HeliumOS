#ifndef HELIUM_BOOTINFO_H
#define HELIUM_BOOTINFO_H

#include <bootboot.h>
#include <mem.h>
#include <stdint.h>

extern char     environment[MEM_PS];

extern BOOTBOOT bootboot;

extern uint8_t  fb;

#endif