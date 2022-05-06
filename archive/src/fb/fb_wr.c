#include <fb.h>

#include <stdbool.h>
#include <string.h>
#include <debug.h>
#include <ctype.h>
#include <utils.h>
#include <lambda.h>

#include "internal_fb.h"

void fb_wr(char const* s, bool flush, ...)
{
    va_list args;
    va_start(args, flush);
    fb_v_wr(s, flush, args);
    va_end(args);
}

void fb_v_wr(char const* s, bool flush, va_list args)
{
    uint32_t curs_y_begin = internal_fb_curs_y;
    for(; *s != 0; ++s)
    {
        switch(*s)
        {
            case '%':
            {
                int  width     = 0    ;
                int precision  = 0    ;
                bool prefix    = false;
                bool sign      = false;
                bool long_mode = false;
                ++s;
                // read flags
                while(*s == '#' || *s == '+')
                {
                    // why tf did I do it thiw way
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
                        usigned = true;
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
                    {
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
                        buf[65] = 0;
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
                    }
                    case 'c':
                    {
                        for(; width > 1; --width)
                            fb_wrc(' ', false);
                        fb_wrc((char) va_arg(args, int), false);
                        break;
                    }
                    case 's':
                    {
                        char const* str = va_arg(args, char const*);
                        int len = precision == 0 ? strlen(str) : precision;
                        width -= len;
                        for(; width > 0; --width)
                            fb_wrc(' ', false);
                        fb_wrm(str, len, false);
                        break;
                    }
                    case 'e':
                    {
                        // this isn't for exponential writing of a float, but to display enum values
                        // it reads a 32/64bit uint(depending on l flag) from the arguments 
                        // and displays text depending on its value
                        // for example "%e0:BAR,2:FOO,#:UNDEF"
                        // would write BAR if arg = 0, or FOO if arg = 2, or UNDEF otherwise
                        // # for else should be at the end and is optional
                        // * can be used to indicate that the enum value is 
                        // passed in the argument list AFTER the enum value
                        // * can also be used to indicate that the string to display is passed 
                        // in the argument list AFTER the enum value
                        // so ("%e0:A,2:E", enum_value) is equivalent to ("%e*:*.*,*:*", enum_value, 0, "A", 2, "E")
                        // # cannot be passed in the argument list as it would be indi
                        ++s;
                        char const* str = 0;
                        uint64_t enum_value;
                        if(long_mode)
                            enum_value = va_arg(args, uint64_t);
                        else
                            enum_value = va_arg(args, uint32_t);
                        while(*s == '*' || *s == '#' || isdigit(*s))
                        {
                            uint64_t switch_val = UINT64_MAX;
                            if(*s == '*')
                            {
                                if(long_mode)
                                    switch_val = va_arg(args, uint64_t);
                                else
                                    switch_val = va_arg(args, uint32_t);
                                ++s;
                            }
                            else if(isdigit(*s))
                                switch_val = stou(s, &s, 10);
                            else if(*s == '#')
                                ++s;
                            if(*(s - 1) == '#' || switch_val == enum_value)
                            {
                                if(*s != ':')
                                {
                                    fb_wrs("EXPECTED ':' FOUND '", false);
                                    fb_wrc(*s, false);
                                    fb_wrc('\'', false);
                                    break;
                                }
                                str = ++s;
                                char const* cmd_end = strpred(
                                    s,
                                    lambda(
                                        int, (int c)
                                        {
                                            return isnalnum(c) && c != ',' && c != ':' && c != '#';
                                        }
                                    )
                                );
                                s = (cmd_end ? cmd_end : s + strlen(s)) - 1;
                                // - 1 cause the for loop will increment s again
                            }
                            else
                            {
                                // we won't print the following string
                                // *s = ':'
                                char const* part_end = strpred(++s, isnalnum);
                                s = part_end ? part_end : s + strlen(s);
                                if(*s == ',')
                                    ++s;
                                else
                                {
                                    --s;
                                    break; // while
                                }
                            }
                        }
                        if(str)
                        {
                            char const* end = strpred(str, isnalnum);
                            width -= end - str;
                        }
                        for(; width > 0; --width)
                            fb_wrc(' ', false);
                        if(str)
                        {
                            while(isalnum(*str))
                            {
                                fb_wrc(*str, false);
                                ++str;
                            }
                        }
                        break;
                    }
                    case 'z':
                    {
                        uint64_t val = 0;
                        if(long_mode)
                            val = va_arg(args, uint64_t);
                        else
                            val = va_arg(args, uint32_t);
                        char buf[33];
                        char* head = buf + 32;
#define UNIT(v, sym) if(v % 1024 != 0) { *(--head) = sym; head = utos(v % 1024, 10, head); }
                        UNIT(UNITS(val), 'B')
                        UNIT(KILOS(val), 'K')
                        UNIT(MEGS (val), 'M')
                        UNIT(GIGS (val), 'G')
                        UNIT(TERAS(val), 'T')
                        UNIT(PETAS(val), 'P')
                        UNIT(EXAS (val), 'E')
#undef UNIT
                        buf[32] = 0;
                        size_t len = strlen(head);
                        width -= len;
                        for(; width > 0; --width)
                            fb_wrc(' ', false);
                        fb_wrs(head, false);
                        break;
                    }
                    case '%':
                        fb_wrc('%', false);
                        break;
                    default:
                        fb_wrs("INVALID % COMMAND!(", false);
                        fb_wrc(*s, false);
                        fb_wrc(')', false);
                        break;
                }
                break;
            }
            default:
                fb_wrc(*s, false);
                break;
        }
    }
    if(flush)
        fb_flush(curs_y_begin * CHRH, (internal_fb_curs_y - curs_y_begin + 1) * CHRH);
}
