#ifndef HELIUM_DEBUG_H
#define HELIUM_DEBUG_H

#define DEBUG_CONSOLE (0xE9)

// The FILE* is just to be compatible with stdio's functions
int dbg_write_chr(char c);
int dbg_write_string(char const *str);

#endif