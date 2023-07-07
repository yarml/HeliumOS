#include <errno.h>
#include <fs.h>
#include <stdlib.h>
#include <string.h>

#include "internal_fs.h"

filesys_llnode *i_fs_head;

filesys *fs_mount(char const *name) {
  if (!fs_valid_sys_name(name)) {
    errno = EINVAL;
    return 0;
  }

  filesys_llnode *newnode = calloc(1, sizeof(filesys_llnode));
  if (!newnode)  // errno set by calloc
    return 0;

  filesys *newfs = &newnode->fs;

  // Allocate root node
  // Not using fs_mkdir because it can complain about empty name
  fsnode *root = calloc(1, sizeof(fsnode));
  if (!root) {
    // free() does not pollute errno as set by calloc
    free(newnode);
    return 0;
  }

  // Setup root node
  root->type = FSNODE_DIR;
  root->fs = newfs;

  // Setup filesystem
  strcpy(newfs->name, name);
  newfs->root = root;

  newnode->next = i_fs_head;
  if (i_fs_head) i_fs_head->prev = newnode;
  i_fs_head = newnode;

  return newfs;
}

void fs_umount(filesys *fs) {
  filesys_llnode *cfsn = i_fs_head;

  while (cfsn) {
    if (&cfsn->fs == fs) break;
    cfsn = cfsn->next;
  }

  if (cfsn->prev) cfsn->prev->next = cfsn->next;
  if (cfsn->next) cfsn->next->prev = cfsn->prev;

  // Call fs cleanup function
  if (fs->impl.fs_release) fs->impl.fs_release(fs);

  // Remove root dir(and all sub nodes)
  fs_rm(fs->root);

  free(fs);
}
