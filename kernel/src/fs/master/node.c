#include <fs.h>
#include <stdlib.h>
#include <string.h>

#include "internal_fs.h"

void fs_rm(fsnode *node) { }

void fs_close(fsnode *node) {
  if (node->refcount) {
    --node->refcount;
  }
  // In the future, I should check if the filesystem has a function to
  // call when refcount reaches 0 so that it can decide if it is time
  // to remove this node from memory
}
