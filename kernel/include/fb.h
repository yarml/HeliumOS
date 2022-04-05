#ifndef HELIUM_FB_H
#define HELIUM_FB_H

#include <stdint.h>
#include <pack.h>

#define PSF_MAGIC (0x864ab572)
 
typedef struct 
{
    uint32_t magic        ;
    uint32_t version      ;
    uint32_t headersize   ;
    uint32_t flags        ;
    uint32_t numglyph     ;
    uint32_t bytesperglyph;
    uint32_t height       ;
    uint32_t width        ;
} pack psf_font;

/** @brief Checks the font and initializes internal variables */
void fb_init();

/**
 * @brief Set a pixel to a color
 * @param c: Color to set
 * @param (x,y): Coordinates of pixel to set
**/
void fb_pxs(uint32_t c, uint32_t x, uint32_t y);

/**
 * @brief Get the color of a pixel at some coordinates
 * @param (x,y): Coordinates if the target pixel
**/
uint32_t fb_pxg(uint32_t x, uint32_t y);

/**
 * @brief Scrolls the text up n line
 * @param n: number of lines to scroll
**/
void fb_scroll(uint32_t n);

/**
 * @brief Writes an ASCII character in specific coordinates
 * @param c: The ASCII character to write
 * @param (x,y): Coordinates of the character in grid position
**/
void fb_putc(char c, uint32_t x, uint32_t y);

/**
 * @brief Writes an ASCII character to the framebuffer
 * @param c: The character to be written
**/
void fb_wrc(char c);

/**
 * @brief Writes a null terminated string to the framebuffer
 * @param s: The string to be written
**/
void fb_wrs(char const* s);

#endif