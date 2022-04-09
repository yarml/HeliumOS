#include <fb.h>

#include <stdbool.h>
#include <string.h>
#include <debug.h>
#include <ctype.h>
#include <math.h>

void fb_wr(char const* s, ...)
{
    va_list args;
    va_start(args, s);
    vfb_wr(s, args);
    va_end(args);
}

void vfb_wr(char const* s, va_list args)
{
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
                        int len = precision == 0 ? strlen(str) : precision;
                        width -= len;
                        for(; width > 0; --width)
                            fb_wrc(' ');
                        fb_wrm(str, len);
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
                break;
            default:
                fb_wrc(*s);
                break;
        }
    }
}
