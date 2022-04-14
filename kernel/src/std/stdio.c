#include <stdio.h>

#include <stdarg.h>
#include <fb.h>


int printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfb_wr(format, true, args);
    va_end(args);
    return 0; // FIXME: return the number of characters written(I'll probably implement it when I need it)
}
