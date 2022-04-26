#include <fb.h>

#include <boot_info.h>
#include <string.h>
#include <debug.h>
#include <utils.h>
#include <stdbool.h>
#include <asm/io.h>

#include "internal_fb.h"

/* Cursor location */
uint32_t internal_fb_curs_x, internal_fb_curs_y;

static uint8_t* dfb = 0;

static bool scroll_flush = false;

void fb_init()
{
    if(font.magic != PSF_MAGIC)
    {
        dbg_draw(0xFF, 0x00, 0x00, 0, 0);
        LOOP;
    }
    internal_fb_curs_x = 0;
    internal_fb_curs_y = 0;

    size_t mmap_size = (bootboot.size - sizeof(BOOTBOOT)) / sizeof(MMapEnt) + 1;
    for(MMapEnt* c = &bootboot.mmap; mmap_size != 0; ++c, --mmap_size)
        if(MMapEnt_IsFree(c) && MMapEnt_Size(c) >= bootboot.fb_height * bootboot.fb_scanline)
        {
            dfb = (uint8_t*) MMapEnt_Ptr(c);
            break;
        }
    if(!dfb)
    {
        dfb = &fb;
        fb_wrs("Couldn't find enough space for double framebuffer!\n", false);
    }
    else
        memset(dfb, 0, bootboot.fb_height * bootboot.fb_scanline);
    fb_wrs("Done initializing text framebuffer.\n", true);
    fb_wr("Double framebuffer at: %p\n", true, dfb);
}

void fb_flush(uint32_t scanline, uint32_t sl_count)
{
    if(dfb != &fb)
    {
        if(!scroll_flush)
            memmove(
                &fb + scanline * bootboot.fb_scanline,
                dfb + scanline * bootboot.fb_scanline,
                sl_count       * bootboot.fb_scanline
                );
        else
        {
            memmove(
                &fb,
                dfb,
                bootboot.fb_height * bootboot.fb_scanline
                );
            scroll_flush = false;
        }
    }
}

void fb_pxs(uint32_t c, uint32_t x, uint32_t y, uint8_t* buf)
{
    *((uint32_t*)(buf + y * bootboot.fb_scanline + x * 4)) = c;
}

uint32_t fb_pxg(uint32_t x, uint32_t y)
{
    return *((uint32_t*)(dfb + y * bootboot.fb_scanline + x * 4));
}

void fb_scroll(uint32_t n, bool flush)
{
    memmove(dfb, dfb + n * CHRH * bootboot.fb_scanline, (FB_HEIGHT - n) * CHRH * bootboot.fb_scanline);
    memset(dfb + (FB_HEIGHT - n) * CHRH * bootboot.fb_scanline, 0, n * CHRH * bootboot.fb_scanline);
    scroll_flush = true;
    if(flush)
        fb_flush(0, bootboot.fb_scanline);
}

void fb_putc(char c, uint32_t dx, uint32_t dy, bool flush)
{
    uint8_t* glyph = ((uint8_t*)&font) + font.headersize + c * font.bytesperglyph;
    uint32_t bpl = (font.width + 7) / 8; // bytes per line
    for(int px = 0; px < font.width; ++px)
        for(int py = 0; py < font.height; ++py)
        {
            uint32_t color = BIT(glyph[py * bpl  + px / 8], 7 - px % 8) ? 0x00FFFFFF : 0x00000000;
            for(int x = 0; x < SCALEX; ++x)
                for(int y = 0; y < SCALEY; ++y)
                    fb_pxs(color, x + px * SCALEX + dx * CHRW, y + py * SCALEY + dy * CHRH, dfb);
        }
    if(flush)
        fb_flush(dy * CHRH, CHRH);
}

void fb_wrc(char c, bool flush)
{
    as_outb(DEBUG_CONSOLE, c);
    uint32_t curs_y_begin = internal_fb_curs_y;
    switch(c)
    {
        case 0: // this case is only useful when f_wrs finds a \ and too lazy to check for a 0 after it
            return;
        case '\n':
            ++internal_fb_curs_y;
            internal_fb_curs_x = 0;
            break;
        case '\t':
            fb_wrs("  ", false);
            break;
        case '\v':
            ++internal_fb_curs_y;
            break;
        case '\r':
            internal_fb_curs_x = 0;
            break;
        case '\b':
            if(!internal_fb_curs_x)
            {
                if(internal_fb_curs_y)
                {
                    internal_fb_curs_x = FB_WIDTH - 1;
                    --internal_fb_curs_y;
                }
            }
            else
                --internal_fb_curs_x;
            fb_putc(' ', internal_fb_curs_x, internal_fb_curs_y, false);
            break;
        case '\f':
            fb_scroll(1, false);
            if(internal_fb_curs_y != 0)
                --internal_fb_curs_y;
            break;
        default:
            fb_putc(c, internal_fb_curs_x, internal_fb_curs_y, false);
            ++internal_fb_curs_x;
            break;
    }
    if(internal_fb_curs_x >= FB_WIDTH)
    {
        internal_fb_curs_x = 0;
        ++internal_fb_curs_y;
    }
    if(internal_fb_curs_y >= FB_HEIGHT)
    {
        fb_scroll(internal_fb_curs_y - FB_HEIGHT + 1, false);
        internal_fb_curs_y = FB_HEIGHT - 1;
    }
    if(flush)
        fb_flush(curs_y_begin * CHRH, (internal_fb_curs_y - curs_y_begin + 1) * CHRH);
}

void fb_wrs(char const* s, bool flush)
{
    uint32_t curs_y_begin = internal_fb_curs_y;
    for(; *s != 0; ++s)
        fb_wrc(*s, false);
    if(flush)
        fb_flush(curs_y_begin * CHRH, (internal_fb_curs_y - curs_y_begin + 1) * CHRH);
}

void fb_wrm(char const* s, size_t len, bool flush)
{
    uint32_t curs_y_begin = internal_fb_curs_y;
    for(; len != 0; --len, ++s)
        fb_wrc(*s, false);
    if(flush)
        fb_flush(curs_y_begin * CHRH, (internal_fb_curs_y - curs_y_begin + 1) * CHRH);
}
