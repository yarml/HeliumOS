#include <debug.h>
#include <string.h>

#include <asm/io.h>

int dbg_write_chr(char c)
{
  as_outb(DEBUG_CONSOLE, c);
  return 0;
}

int dbg_write_string(char const *str)
{
  as_outsb(
    DEBUG_CONSOLE,
    (uint8_t *) str,
    strlen(str)
  );
  return 0;
}
