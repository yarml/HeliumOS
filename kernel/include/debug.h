#ifndef HELIUM_DEBUG_H
#define HELIUM_DEBUG_H

#include <stdint.h>
#include <stdio.h>

/**
 * \brief Puts the thread in an infinite loop(with no way out!).
 */
#define LOOP while(1)

#define DEBUG_CONSOLE (0xE9)

/**
 * \brief Draws a square of color (\a r, \a g, \a b) at (\a offx, \a offy) for debugging purposes.
 * \param r
 * \param g
 * \param b
 * \param offx
 * \param offy
 */
void dbg_draw(uint8_t r, uint8_t g, uint8_t b, uint32_t offx, uint32_t offy);

// The FILE* is just to be compatible with stdio's functions
int dbg_write_chr(FILE* f, char c);
int dbg_write_string(FILE*, char const* str);

FILE dbg_output_file();

#endif