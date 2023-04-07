#ifndef HELIUM_INT_FS_H
#define HELIUM_INT_FS_H

#include <fs.h>

typedef struct FILESYS_LLNODE filesys_llnode;

struct FILESYS_LLNODE
{
  filesys fs;

  filesys_llnode *next;
  filesys_llnode *prev;
};

extern filesys_llnode *i_fs_head;

void debug_initfs();

#endif