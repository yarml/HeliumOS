#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stddef.h>
#include <stdarg.h>

struct __stdio__file;
typedef struct __stdio__file FILE;
struct __stdio__file
{
    int (*write_chr)(FILE* f, char c);
    int (*write_string)(FILE* f, char const* str);
};

extern FILE* stdout;

int printf(const char* template, ...);
int vprintf(char const* template, va_list va);

int fprintf(FILE* stream, char const* template, ...);
int vfprintf(FILE* stream, char const* template, va_list va);

int snprintf(char* s, size_t size, char const* template, ...);
int vsnprintf(char* s, size_t size, char const* template, va_list va);

#endif