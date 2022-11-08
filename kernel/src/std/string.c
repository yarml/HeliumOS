#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <utils.h>
#include <math.h>

#include <asm/movs.h>
#include <asm/stos.h>
#include <asm/scas.h>

#define MASS_OP_MIN_BYTES (64)

size_t strlen(char const *s)
{
    return -as_scasb((uint64_t) s, 0, -1) - 2;
}

char *strchr(char const *s, int c)
{
    /* TODO: Idk why I didn't switch this to use memchr, wanted to change it
        But I'm afraid of breaking it. Figure this out later. */
    for(; *(unsigned char*)s != c; ++s)
        if(*s == 0) 
            return 0;
    return (char*) s;
}

char *strcat(char* to, char const* from)
{
    strcpy(to + strlen(to), from);
    return to;
}

char *strcpy(char *to, char const* from)
{
    return memcpy(to, from, strlen(from) + 1);
}

char *strpred(char const *s, fpt_chr_predicate pred)
{
    for(; !pred(*s); ++s)
        if(*s == 0) 
            return 0;
    return (char*) s;
}

void *memchr (void const *block, int c, size_t size)
{
    size_t idx = size - as_scasb((uint64_t) block, c, size) - 1;
    return (((unsigned char*)block)[idx] == c) ? (void*) block + idx : 0;
}

void *memnchr(void const *block, int c, size_t size)
{
    register size_t idx = size - as_nscasb((uint64_t) block, c, size) - 1;
    if(idx != size - 1)
        return (void*) block + idx;
    return (((unsigned char*)block)[idx] != c) ? (void*) block + idx : 0;
}

int memcmp (void const *b1, void const *b2, size_t size)
{
    for(; *(unsigned char*)b1 == *(unsigned char*)b2 && size != 0; --size, ++b1, ++b2);
    return *(unsigned char*)(b2 - 1) - *(unsigned char*)(b1 - 1);
}

void *memset(void *block, int c, size_t size)
{
    void* oblock = block;


    while((uintptr_t)block % 8 && size)
    {
        *(uint8_t*)block++ = c;
        --size;
    }
    if(size >= MASS_OP_MIN_BYTES)
    {
        uint64_t c64 = c;
        c64 = c64 << 8  | c64;
        c64 = c64 << 16 | c64;
        c64 = c64 << 32 | c64;
        as_stosq((uint64_t)block, c64, size / 8);
        block += size / 8;
        size -= size / 8;
    }
    as_stosb((uint64_t)block, c, size);

    return oblock;
}

void *memcpy(void *to, void const *from, size_t size)
{
    void *oto = to;

    // TODO: Maybe max_alignment can be calculated in constant time
    size_t max_alignment = 1;

    if((uintptr_t) to % 8 == (uintptr_t)from % 8)
        max_alignment = 8;
    else if((uintptr_t)to % 4 == (uintptr_t)from % 4)
        max_alignment = 4;
    else if((uintptr_t)to % 2 == (uintptr_t)from % 2)
        max_alignment = 2;

    // copy individual bytes until both from and to are aligned to max_alignment
    // at worst case, it should individually copy 7 bytes
    while(((uintptr_t)to % max_alignment || (uintptr_t)from % max_alignment) && size)
    {
        *(uint8_t *)to++ = *(uint8_t*)from++;
        --size;
    }
    // Mass copy the now aligned bytes if there exist enough of them for mass copying to be efficient
    if(size >= MASS_OP_MIN_BYTES)
    {
        void(*movsfp)(uint64_t, uint64_t, uint64_t);   
        switch(max_alignment)
        {
            case 8:
                movsfp = as_movsq;
                break;
            case 4:
                movsfp = as_movsd;
                break;
            case 2:
                movsfp = as_movsw;
                break;
            default:
                movsfp = as_movsb;
                break;
        }
        movsfp((uint64_t)to, (uint64_t)from, size / max_alignment);
        to += ALIGN_DN(size, max_alignment);
        from += ALIGN_DN(size, max_alignment);
        size -= ALIGN_DN(size, max_alignment);
    }
    // Copy leftover bytes, should be a maximum of 7
    as_movsb((uint64_t)to, (uint64_t)from, size);

    return oto;
}


void *memmove(void *to, void const *from, size_t size)
{
    if(to < from || from + size < to)
    {
        return memcpy(to, from, size);
    }

    // save to for return
    void* oto = to;

    to += size - 1;
    from += size - 1;

    // TODO: Maybe max_alignment can be calculated in constant time
    size_t max_alignment = 1;

    if((uintptr_t) to % 8 == (uintptr_t)from % 8)
        max_alignment = 8;
    else if((uintptr_t)to % 4 == (uintptr_t)from % 4)
        max_alignment = 4;
    else if((uintptr_t)to % 2 == (uintptr_t)from % 2)
        max_alignment = 2;

    // copy individual bytes until both from and to are aligned to max_alignment
    // at worst case, it should individually copy 7 bytes
    while(((uintptr_t)to % max_alignment || (uintptr_t)from % max_alignment) && size)
    {
        *(uint8_t*)to-- = *(uint8_t*)from--;
        --size;
    }
    // Mass copy the now aligned bytes if there exist enough of them for mass copying to be efficient
    if(size >= MASS_OP_MIN_BYTES)
    {
        void(*movsfp)(uint64_t, uint64_t, uint64_t);   
        switch(max_alignment)
        {
            case 8:
                movsfp = as_rmovsq;
                break;
            case 4:
                movsfp = as_rmovsd;
                break;
            case 2:
                movsfp = as_rmovsw;
                break;
            default:
                movsfp = as_rmovsb;
                break;
        }
        movsfp((uint64_t)to, (uint64_t)from, size / max_alignment);
        to -= ALIGN_DN(size, max_alignment);
        from -= ALIGN_DN(size, max_alignment);
        size -= ALIGN_DN(size, max_alignment);
    }
    // Copy leftover bytes, should be a maximum of 7
    as_rmovsb((uint64_t)to, (uint64_t)from, size);

    return oto;
}


int memsum(void *block, size_t size)
{
    int8_t sum = 0;
    for(; size != 0; --size, ++block)
        sum += *(int8_t*)block;
    return sum;
}

char* ntos(intmax_t n, int base, char *tail){
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


char *utos(uintmax_t n, int base, char *tail)
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

intmax_t ston(char const *s, char const **tail, int base)
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

uintmax_t stou(char const *s, char const **tail, int base)
{
    return ston(s, tail, base);
}

