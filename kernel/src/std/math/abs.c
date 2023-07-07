#include <stdint.h>

uintmax_t abs(intmax_t i) { return i < 0 ? -i : i; }
