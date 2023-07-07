#include <stddef.h>

#include <asm/scas.h>

size_t strlen(char const *s) { return -as_scasb((uint64_t)s, 0, -1) - 2; }
