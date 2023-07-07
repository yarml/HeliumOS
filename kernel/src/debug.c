#include <debug.h>
#include <string.h>

#include <asm/io.h>

int dbg_write_chr(char c) {
  as_outb(DEBUG_CONSOLE, c);
  return c;
}

int dbg_write_string(char const *str) {
  size_t str_len = strlen(str);
  as_outsb(DEBUG_CONSOLE, (uint8_t *)str, str_len);
  return str_len;
}
