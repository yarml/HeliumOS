#include <fb.h>
#include <boot_info.h>
#include <debug.h>
#include <utils.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

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

    //fb_wrs("Ini");
    fb_wrs("Done initializing text framebuffer.\n");
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
        for(int y = 0; y < (FB_HEIGHT - n) * CHRH; ++y)
            fb_pxs(fb_pxg(x, y + n * CHRH), x, y);

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

void fb_wr(char const* s, ...)
{
    va_list args;
    va_start(args, s);
    vfb_wr(s, args);
    va_end(args);
}

void prd(uintmax_t i)
{
    char buf[34];
    fb_wrs(utos(i, 10, buf + 33));
}

void vfb_wr(char const* s, va_list args)
{
    for(; *s != 0; ++s)
    {
        switch(*s)
        {
            case '%':
                int  width     = 0    ;
                bool prefix    = false;
                bool sign      = false;
                bool long_mode = false;
                ++s;
                read_flag:
                switch(*s)
                {
                    case '#':
                        prefix = true;
                        break;
                    case '+':
                        sign = true;
                        break;
                    default:
                        goto read_width;
                }
                ++s;
                goto read_flag;
                read_width:
                if(*s == '*')
                {
                    width = va_arg(args, int);
                    ++s;
                    goto read_length;
                }
                char* lsd = strpred(s, isndigit) - 1;
                if(s > lsd)
                    goto read_length;
                for(; s <= lsd; ++s)
                    width += (*s - '0') * exp10i(lsd - s);
                read_length:
                if(*s == 'l')
                {
                    long_mode = true;
                    ++s;
                }
                // read_specifier
                int base = 0;
                bool usigned = false;
                switch(*s)
                {
                    case 'd':
                    case 'i':
                        base = 10;
                        goto print_num;
                    case 'b':
                        base = 2;
                        goto print_num;
                    case 'u':
                        base = 10;
                        usigned = true;
                        goto print_num;
                    case 'o':
                        base = 8;
                        usigned = true;
                        goto print_num;
                    case 'x':
                        base = 16;
                        usigned = true;
                        goto print_num;
                    print_num:
                        char* result = 0;
                        char buf[66];
                        if(long_mode)
                        {
                            if(usigned)
                                result = utos(va_arg(args, unsigned long long int), base, buf + 65);
                            else
                                result = ntos(va_arg(args, long long int), base, buf + 65);
                        }
                        else
                        {
                            if(usigned)
                                result = utos(va_arg(args, unsigned int), base, buf + 65);
                            else
                                result = ntos(va_arg(args, int), base, buf + 65);
                        }
                        if(prefix)
                        {
                            switch(base)
                            {
                                case 10:
                                    fb_wrs("0d");
                                    break;
                                case 2:
                                    fb_wrs("0b");
                                    break;
                                case 8:
                                    fb_wrs("0c");
                                    break;
                                case 16:
                                    fb_wrs("0x");
                                    break;
                                default:
                                    fb_wrs("0u");
                                    break;
                            }
                        }
                        if(sign && isdigit(*result) && base == 10)
                            fb_wrc('+');
                        if(isnalnum(*result))
                        {
                            fb_wrc('-');
                            ++result;
                        }
                        width -= strlen(result);
                        
                        for(; width > 0; --width)
                            fb_wrc(base == 10 ? ' ' : '0');
                        fb_wrs(result);
                        break;
                    case 'c':
                        for(; width > 1; --width)
                            fb_wrc(' ');
                        fb_wrc((char) va_arg(args, int));
                        break;
                    case 's':
                        char const* str = va_arg(args, char const*);
                        width -= strlen(str);
                        for(; width > 0; --width)
                            fb_wrc(' ');
                        fb_wrs(str);
                        break;
                    case 'p':
                        void* ptr = va_arg(args, void*);
                        if(width > 0)
                        {
                            char command[15];
                            char* w = utos(width, 10, command + 12);
                            *(w - 1) = '%';
                            command[12] = 'l';
                            command[13] = 'x';
                            command[14] = 0  ;
                            fb_wr(w - 1, (unsigned long long int) ptr);
                        }
                        else
                            fb_wr("%lx", (unsigned long long int) ptr);
                        break;
                    case '%':
                        fb_wrc('%');
                        break;
                    default:
                        fb_wrs("INVALID % COMMAND!(");
                        fb_wrc(*s);
                        fb_wrc(')');
                        LOOP;
                        break;
                    }
                ++s;
            default:
                fb_wrc(*s);
                break;
        }
    }
}
