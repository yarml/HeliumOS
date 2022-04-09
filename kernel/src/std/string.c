#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

size_t strlen(char const* s)
{
    size_t len = 0;
    for(; *s != 0; ++s)
        ++len;
    return len;
}
char* strchr(char const* s, int c)
{
    for(; *s != c; ++s)
        if(*s == 0) 
            return 0;
    return (char*) s;
}

char* strpred(char const* s, fpt_chr_predicate pred)
{
    for(; !pred(*s); ++s)
        if(*s == 0) 
            return 0;
    return (char*) s;
}

void* memchr (const void* block, int c, size_t size)
{
    for(; size != 0; --size, ++block)
        if(*(char*) block == c) 
            return (void*) block;
    return 0;
}

int memcmp (const void* b1, const void* b2, size_t size)
{
    for(; *(char*)b1 == *(char*)b2 && size != 0; --size, ++b1, ++b2);
    return *(char*)(b2 - 1) - *(char*)(b1 - 1);
}

void* memset(void* block, int c, size_t size)
{
    void* org_block = block;
    for(; size != 0; --size, ++block)
        *(char*)block = c;
    return org_block;;
}

void* memcpy(void* to, const void* from, size_t size)
{
    void* org_to = to;
    for(; size != 0; --size, ++to, ++from)
        *(char*)to = *(char*)from;
    return org_to;
}

void* memmove(void* to, void const* from, size_t size)
{
    if(from == to || size == 0) // These arent really special cases, I just think its more natural to skip them
        return to;
    int direction = (from - to) / abs(from - to);
    uint8_t const* from_org = direction == -1 ? from + size - 1 : from;
    uint8_t*       to_org   = direction == -1 ? to   + size - 1 : to  ;
    while(size != 0)
    {
        *to_org = *from_org;
        --size;
        from_org += direction;
        to_org += direction;
    }
    return to;
}

int memsum(void* block, size_t size)
{
    int8_t sum = 0;
    for(; size != 0; --size, ++block)
        sum += *(int8_t*)block;
    return sum;
}

char* ntos(intmax_t n, int base, char* null){
    bool negative = n < 0;
    *null = 0;
    do
    {
        --null;
        if(abs(n % base) < 10)
            *null = '0' + abs(n % base);
        else
            *null = 'A' + abs(n % base - 10);
        n /= base;
    } while(n != 0);
    if(negative)
    {
        --null;
        *null = '-';
    }
    return null;
}


char* utos(uintmax_t n, int base, char* null)
{
    *null = 0;
    do
    {
        --null;
        if(n % base < 10)
            *null = '0' + n % base;
        else
            *null = 'A' + n % base - 10;
        n /= base;
    } while(n != 0);
    return null;
}

intmax_t ston(char const* s, char const** tail, int base)
{
    intmax_t res = 0;
    char* lsd = strpred(s, isndigit) - 1;
    if(tail)
        *tail = lsd + 1;
    if(s > lsd)
        return 0;
    for(; s <= lsd; ++s)
        res += (*s - '0') * powi(base, lsd - s);
    return res;    
}

uintmax_t stou(char const* s, char const** tail, int base)
{
    return ston(s, tail, base);
}

