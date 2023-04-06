#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stddef.h>
#include <stdarg.h>
#include <fs.h>

#define EOF (int)(-1)

typedef struct STDIO_FILE FILE;
struct STDIO_FILE
{
  fsnode *fnode;

  int mode;

  size_t cur; // cursor
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

// File functions
FILE *fopen(char *path, char *mode);
int fclose(FILE *stream);

FILE *__get_stdout();
FILE *__get_stderr();

#define stdout (__get_stdout())
#define stderr (__get_stderr())

#endif