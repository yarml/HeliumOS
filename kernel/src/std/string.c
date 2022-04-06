#include <stdbool.h>
#include <stdint.h>
#include <string.h>
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

char* strpred(char const* s, chr_predicate pred)
{
    for(; !pred(*s); ++s)
        if(*s == 0) 
            return 0;
    return (char*) s;
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
