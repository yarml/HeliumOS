#include <stdint.h>

intmax_t powi(intmax_t base, uintmax_t power) {
  intmax_t result = 1;
  while (1) {
    if (power & 1) {
      result *= base;
    }
    power >>= 1;
    if (!power) {
      break;
    }
    base *= base;
  }
  return result;
}
