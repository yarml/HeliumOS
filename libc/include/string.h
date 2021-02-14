#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

size_t strlen(const char* str                                          );
void*  memcpy(void* to, const void* from, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* LIBC_STRING_H */