#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stdarg.h>
#include <stddef.h>

#define EOF (int)(-1)

// Prints to debug console
int printd(char const *template, ...);
int vprintd(char const *template, va_list va);

// Prints to kernel terminal
int printf(char const *template, ...);
int vprintf(char const *template, va_list va);

int snprintf(char *s, size_t size, char const *template, ...);
int vsnprintf(char *s, size_t size, char const *template, va_list va);

int prtrace_begin(char const *fname, char const *args, ...);
int prtrace_end(char const *fname, char const *status, char const *result, ...);

int putchar(int c);

int puts(char const *s);

/* Temporary printd, used when in the middle of implementing
   a feature to debug out internal information */
int tpd(char const *template, ...);
/* Temporary printf, like tpd but for printf instead of printd */
int tpf(char const *template, ...);

int    getc();
size_t getdelim(char **restrict lineptr, size_t *restrict n, int delim);
size_t getline(char **restrict lineptr, size_t *restrict n);

#endif