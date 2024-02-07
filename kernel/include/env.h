#ifndef HELIUM_ENV_H
#define HELIUM_ENV_H

#include <stdbool.h>
#include <stdint.h>

// Not 100% accurate, works with Qemu
bool     env_isvm();
uint32_t env_busfreq();

#endif