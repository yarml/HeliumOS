#include <fb.h>

#include <boot_info.h>
#include <debug.h>
#include <utils.h>
#include <io.h>

extern psf_font _binary_kernel__font_psf_start;

#define font (_binary_kernel__font_psf_start)

#define SCALEX (2)
#define SCALEY (2)

#define CHRW (font.width  * SCALEX)
#define CHRH (font.height * SCALEY)

#define FB_WIDTH  (bootboot.fb_width  / CHRW)
#define FB_HEIGHT (bootboot.fb_height / CHRH)


/* Cursor location */
static uint32_t curs_x, curs_y;

void fb_init()
{
    if(font.magic != PSF_MAGIC)
    {
        dbg_draw(0xFF, 0x00, 0x00, 0, 0);
        LOOP;
    }
    curs_x = 0;
    curs_y = 0;

    fb_wrs("Done initializing text framebuffer.\n");;
}

void fb_pxs(uint32_t c, uint32_t x, uint32_t y)
{
    *((uint32_t*)(&fb + y * bootboot.fb_scanline + x * 4)) = c;
}

uint32_t fb_pxg(uint32_t x, uint32_t y)
{
    return *((uint32_t*)(&fb + y * bootboot.fb_scanline + x * 4));
}

void fb_scroll(uint32_t n)
{
    for(int x = 0; x < bootboot.fb_width; ++x)
    {
        for(int y = 0; y < (FB_HEIGHT - n) * CHRH; ++y)
            fb_pxs(fb_pxg(x, y + n * CHRH), x, y);
        for(int y = (FB_HEIGHT - n) * CHRH; y < FB_HEIGHT * CHRH; ++y)
            fb_pxs(0, x, y);
    }
}

void fb_putc(char c, uint32_t dx, uint32_t dy)
{
    uint8_t* glyph = ((uint8_t*)&font) + font.headersize + c * font.bytesperglyph;
    uint32_t bpl = (font.width + 7) / 8; // bytes per line
    
    for(int px = 0; px < font.width; ++px)
        for(int py = 0; py < font.height; ++py)
        {
            uint32_t color = BIT(glyph[py * bpl  + px / 8], 7 - px % 8) ? 0x00FFFFFF : 0x00000000;
            for(int x = 0; x < SCALEX; ++x)
                for(int y = 0; y < SCALEY; ++y)
                    fb_pxs(color, x + px * SCALEX + dx * CHRW, y + py * SCALEY + dy * CHRH);
        }
}

void fb_wrc(char c)
{
    switch(c)
    {
        case 0: // this case is only useful when f_wrs finds a \ and too lazy to check for a 0 after it
            return;
        case '\n':
            ++curs_y;
            curs_x = 0;
            break;
        case '\t':
            fb_wrs("  ");
            break;
        case '\v':
            ++curs_y;
            break;
        case '\r':
            curs_x = 0;
            break;
        case '\f':
            fb_scroll(1);
            if(curs_y != 0)
                --curs_y;
            break;
        default:
            fb_putc(c, curs_x, curs_y);
            ++curs_x;
            break;
    }
    if(curs_x >= FB_WIDTH)
    {
        curs_x = 0;
        ++curs_y;
    }
    if(curs_y >= FB_HEIGHT)
    {
        fb_scroll(curs_y - FB_HEIGHT + 1);
        curs_y = FB_HEIGHT - 1;
    }

}

void fb_wrs(char const* s)
{
    for(; *s != 0; ++s)
        fb_wrc(*s);
}

void fb_wrm(char const* s, size_t len)
{
    for(; len != 0; --len, ++s)
        fb_wrc(*s);
        
}
