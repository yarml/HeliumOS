#ifndef HELIUM_CLR_DBG_H
#define HELIUM_CLR_DBG_H

#include <stdint.h>

#define LOOP while(1)

/**
 * @brief Draws colored suqres used for debugging with colors consistent along all framebuffer types. 
 * @param r: The red component of the color
 * @param g: The green component of the color
 * @param b: the blue component of the color
 * @param offx: The X offset of the square in grid unit
 * @param offy: The Y offset of the square in grid unit
**/
void dbg_draw(uint8_t r, uint8_t g, uint8_t b, uint32_t offx, uint32_t offy);

#endif