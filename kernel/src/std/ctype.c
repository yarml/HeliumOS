#include <ctype.h>


int isdigit(int c)
{
    return '0' <= c && c <= '9';
}

int isndigit(int c)
{
    return c < '0' || '9' < c;
}

int isbdigit(int c, int base)
{
    return ('0' <= c && c < '0' + (base < 10 ? base : 10))
        || ('A' <= c && c < 'A' +  base - 10)
        || ('a' <= c && c < 'a' +  base - 10);
}

int isnbdigit(int c, int base)
{
    return !isbdigit(c, base);
}

int isupper(int c)
{
    return 'A' <= c && c <= 'Z';
}

int isnupper(int c)
{
    return c < 'A' || 'Z' < c;
}


int islower(int c)
{
    return 'a' <= c && c <= 'z';
}

int isnlower(int c)
{
    return c < 'a' || 'z' < c;
}


int isalpha(int c)
{
    return ('A' <= c && c <= 'Z')
        || ('a' <= c && c <= 'z');
}

int isnalpha(int c)
{
    return (c < 'A' || 'Z' < c)
        && (c < 'a' || 'z' < c);
}


int isalnum(int c)
{
    return ('0' <= c && c <= '9')
        || ('A' <= c && c <= 'Z')
        || ('a' <= c && c <= 'z');
}

int isnalnum(int c)
{
    return (c < '0' || '9' < c)
        && (c < 'A' || 'Z' < c)
        && (c < 'a' || 'z' < c);
}
