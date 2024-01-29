#ifndef HELIUM_INITRD_H
#define HELIUM_INITRD_H

#include <stddef.h>

#define INITRD_VPTR ((void *)(0xFFFF810000000000))

typedef struct INITRD_FILE {
  char const *path;
  size_t      size;
  char        type;
  void       *content;  // Only for files
} initrd_file;

void         initrd_init();
initrd_file *initrd_search(char const *path);

#endif