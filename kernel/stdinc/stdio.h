#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stddef.h>
#include <stdarg.h>

#define EOF (int)(-1)

struct __stdio__file;
typedef struct __stdio__file FILE;
struct __stdio__file
{
  int (*write_chr)(FILE* f, char c);
  int (*write_string)(FILE* f, char const* str);
};

extern FILE* stdout;
extern FILE* stderr;

int printf(char const* template, ...);
int vprintf(char const* template, va_list va);

int fprintf(FILE* stream, char const* template, ...);
int vfprintf(FILE* stream, char const* template, va_list va);

int snprintf(char* s, size_t size, char const* template, ...);
int vsnprintf(char* s, size_t size, char const* template, va_list va);

int prtrace_begin(char const *fname, char const *args, ...);
int prtrace_end(char const *fname, char const *status, char const *result, ...);

int fputc(int c, FILE* stream);
int putchar(int c);

// idk they said it's usually implemented as a macro
#define putc(c, s) (s->write_chr(s, c) ? EOF : c)

int fputs(char const* s, FILE* stream);
int puts(char const* s);

/* Temporary printf, used when in the middle of implementing
   a feature to debug out internal information */
int tpf(char const* template, ...);

#endif