#include <cpuid.h>
#include <env.h>

bool env_isvm() {
  uint32_t a, b, c, d;
  __cpuid(1, a, b, c, d);
  return c & (1 << 31);
}

uint32_t env_busfreq() {
  if (env_isvm()) {
    // If running in a VM, will just set bus frequency to 100MHz, accuracy
    // doesn't matter in this context.
    return 100;
  }
  uint32_t a, b, c, d;
  __cpuid(0x16, a, b, c, d);
  return c;
}
