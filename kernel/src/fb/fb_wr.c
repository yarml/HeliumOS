#include <fb.h>

#include <stdbool.h>
#include <string.h>
#include <debug.h>
#include <ctype.h>
#include <math.h>

#include "internal_fb.h"

void fb_wr(char const* s, bool flush, ...)
{
    va_list args;
    va_start(args, flush);
    vfb_wr(s, flush, args);
    va_end(args);
}

void vfb_wr(char const* s, bool flush, va_list args)
{
    uint32_t curs_y_begin = internal_fb_curs_y;
    for(; *s != 0; ++s)
    {
        switch(*s)
        {
            case '%':
                int  width     = 0    ;
                int precision  = 0    ;
                bool prefix    = false;
                bool sign      = false;
                bool long_mode = false;
                ++s;
                // read flags
                while(*s == '#' || *s == '+')
                {
                    bool* target = *s == '#' ? &prefix : &sign;
                    *target = true;
                    ++s;
                }
                // read width
                if(*s == '*')
                {
                    width = va_arg(args, int);
                    ++s;
                }
                else
                    width = stou(s, &s, 10);
                // read precision
                if(*s == '.')
                {
                    ++s;
                    if(*s == '*')
                    {
                        precision = va_arg(args, int);
                        ++s;
                    }
                    else
                        precision = ston(s, &s, 10);
                }
                // read length
                if(*s == 'l')
                {
                    long_mode = true;
                    ++s;
                }
                // read specifier
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
                    case 'p':
                        base = 16;
                        usigned = true;
                        long_mode = true;
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
                                    fb_wrs("0d", false);
                                    break;
                                case 2:
                                    fb_wrs("0b", false);
                                    break;
                                case 8:
                                    fb_wrs("0c", false);
                                    break;
                                case 16:
                                    fb_wrs("0x", false);
                                    break;
                                default:
                                    fb_wrs("0u", false);
                                    break;
                            }
                        }
                        if(sign && isdigit(*result) && base == 10)
                            fb_wrc('+', false);
                        if(isnalnum(*result))
                        {
                            fb_wrc('-', false);
                            ++result;
                        }
                        width -= strlen(result);
                        
                        for(; width > 0; --width)
                            fb_wrc(base == 10 ? ' ' : '0', false);
                        fb_wrs(result, false);
                        break;
                    case 'c':
                        for(; width > 1; --width)
                            fb_wrc(' ', false);
                        fb_wrc((char) va_arg(args, int), false);
                        break;
                    case 's':
                        char const* str = va_arg(args, char const*);
                        int len = precision == 0 ? strlen(str) : precision;
                        width -= len;
                        for(; width > 0; --width)
                            fb_wrc(' ', false);
                        fb_wrm(str, len, false);
                        break;
                    case '%':
                        fb_wrc('%', false);
                        break;
                    default:
                        fb_wrs("INVALID % COMMAND!(", false);
                        fb_wrc(*s, false);
                        fb_wrc(')', false);
                        LOOP;
                        break;
                    }
                break;
            default:
                fb_wrc(*s, false);
                break;
        }
    }
    if(flush)
        fb_flush(curs_y_begin * CHRH, (internal_fb_curs_y - curs_y_begin + 1) * CHRH);
}
