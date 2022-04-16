#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <asm/movs.h>
#include <asm/stos.h>

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

#define QLEN(size)    ((size                                                   ) / 8)
#define DLEN(size)    ((size - 8 * QLEN(size)                                  ) / 4)
#define WLEN(size)    ((size - 8 * QLEN(size) - 4 * DLEN(size)                 ) / 2)
#define BLEN(size)    ((size - 8 * QLEN(size) - 4 * DLEN(size) - 2 * WLEN(size)) / 1)

#define QORG(h) (uint64_t) (     h                  )
#define DORG(h) (uint64_t) (QORG(h) + 8 * QLEN(size))
#define WORG(h) (uint64_t) (DORG(h) + 4 * DLEN(size))
#define BORG(h) (uint64_t) (WORG(h) + 2 * WLEN(size))

#define RQORG(h, size) (uint64_t) (     (h + size) - 1             )
#define RDORG(h, size) (uint64_t) (RQORG(h,  size) - 8 * QLEN(size))
#define RWORG(h, size) (uint64_t) (RDORG(h,  size) - 4 * DLEN(size))
#define RBORG(h, size) (uint64_t) (RWORG(h,  size) - 2 * WLEN(size))

#define BBYTE(c) ((uint8_t ) (                                         (c)))
#define WBYTE(c) ((uint16_t) ((c << 8 )                         | BBYTE(c)))
#define DBYTE(c) ((uint32_t) (((uint32_t) c << 24) | ((uint32_t) c << 16)             | WBYTE(c)))
#define QBYTE(c) ((uint64_t) (((uint64_t) c << 56) | ((uint64_t) c << 48) | ((uint64_t) c << 40)| ((uint64_t) c << 32) | DBYTE(c)))

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
    as_stosq(QORG(block), QBYTE(c), QLEN(size));
    as_stosd(DORG(block), DBYTE(c), DLEN(size));
    as_stosw(WORG(block), WBYTE(c), WLEN(size));
    as_stosb(BORG(block), BBYTE(c), BLEN(size));

    return block;
}


void* memcpy(void* to, const void* from, size_t size)
{
    as_movsq(QORG(to), QORG(from), QLEN(size));
    as_movsd(DORG(to), DORG(from), DLEN(size));
    as_movsw(WORG(to), WORG(from), WLEN(size));
    as_movsb(BORG(to), BORG(from), BLEN(size));
    return to;
}

#include <stdio.h>

void* memmove(void* to, void const* from, size_t size)
{
    if(to < from || from + size < to)
    {
        as_movsq(QORG(to), QORG(from), QLEN(size));
        as_movsd(DORG(to), DORG(from), DLEN(size));
        as_movsw(WORG(to), WORG(from), WLEN(size));
        as_movsb(BORG(to), BORG(from), BLEN(size));
        return to;
    }

/*     as_rmovsq(QORG(to), QORG(from), QLEN(size));
    as_rmovsd(DORG(to), DORG(from), DLEN(size));
    as_rmovsw(WORG(to), WORG(from), WLEN(size));
    as_rmovsb(BORG(to), BORG(from), BLEN(size)); */

    // TODO: I don't know why the above code doesn't work, using movsb for now
    as_rmovsb((uint64_t) (to + size - 1), (uint64_t) (from + size - 1) , size);

    return to;
}


int memsum(void* block, size_t size)
{
    int8_t sum = 0;
    for(; size != 0; --size, ++block)
        sum += *(int8_t*)block;
    return sum;
}

#undef QLEN
#undef DLEN
#undef WLEN
#undef BLEN

#undef QORG
#undef DORG
#undef WORG
#undef BORG

#undef RQORG
#undef RDORG
#undef RWORG
#undef RBORG

char* ntos(intmax_t n, int base, char* tail){
    bool negative = n < 0;
    do
    {
        --tail;
        if(abs(n % base) < 10)
            *tail = '0' + abs(n % base);
        else
            *tail = 'A' + abs(n % base - 10);
        n /= base;
    } while(n != 0);
    if(negative)
    {
        --tail;
        *tail = '-';
    }
    return tail;
}


char* utos(uintmax_t n, int base, char* tail)
{
    do
    {
        --tail;
        if(n % base < 10)
            *tail = '0' + n % base;
        else
            *tail = 'A' + n % base - 10;
        n /= base;
    } while(n != 0);
    return tail;
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

