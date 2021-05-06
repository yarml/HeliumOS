#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

size_t strlen(const char* str);
void*  memcpy(void* to, const void* from, size_t size);
char*  strchr(const char* string, int c);
uint32_t chrpos(const char* str, char c);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* to, const char* from);

#ifdef __cplusplus
}
#endif

#endif /* LIBC_STRING_H */