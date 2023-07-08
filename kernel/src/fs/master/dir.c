#include <errno.h>
#include <fs.h>
#include <stdlib.h>
#include <string.h>

int fs_check_dcap(fsnode *dir, int cap) {
  int fsu = (dir->fs->dir_cap & FSCAP_USED) != 0;
  int fsc = (dir->fs->dir_cap & cap) != 0;

  int du  = (dir->dir.cap & FSCAP_USED) != 0;
  int dc  = (dir->dir.cap & cap) != 0;

  return ((fsc & dc) | (~fsu & du & dc) | (fsu & fsc) | (fsc & ~du)) & 1;
}

fsnode *fs_mknode(fsnode *parent, char *name) {
  if (!fs_check_dcap(parent, FSCAP_DCREAT)) {
    errno = EOPNOTSUPP;
    return 0;
  }

  if (strlen(name) >= FSNODE_NAMELEN) {
    errno = ENAMETOOLONG;
    return 0;
  }

  fsnode *n = calloc(1, sizeof(fsnode));
  if (!n) {  // errno set by calloc
    return 0;
  }

  strcpy(n->name, name);
  n->fs     = parent->fs;

  n->parent = parent;

  n->nsib   = parent->dir.fchild;
  if (parent->dir.fchild) {
    parent->dir.fchild->psib = n;
  }

  parent->dir.fchild = n;

  return n;
}
fsnode *fs_mkdir(fsnode *parent, char *name) {
  fsnode *dir = fs_mknode(parent, name);
  if (!dir) {
    return 0;
  }

  dir->type = FSNODE_DIR;
  return dir;
}
fsnode *fs_mkfile(fsnode *parent, char *name) {
  fsnode *file = fs_mknode(parent, name);
  if (!file) {
    return 0;
  }
  file->type = FSNODE_FILE;
  return file;
}
fsnode *fs_mklink(fsnode *parent, char *name, fsnode *target) {
  fsnode *link = fs_mknode(parent, name);
  if (!link) {
    return 0;
  }
  link->type        = FSNODE_LINK;

  link->link.target = target;

  if (target) {
    link->link.nlink = target->flink;
    if (target->flink) {
      target->flink->link.plink = link;
    }
    target->flink = link;
  }

  return link;
}

fsnode *fs_nextnode(fsnode *dir, fsnode *current) {
  // Implementation not finished, some filesystems need
  // to first load some resource from a device and update their
  // fs tree before reading from the fs tree
  if (!fs_check_dcap(dir, FSCAP_DLIST)) {
    errno = EOPNOTSUPP;
    return 0;
  }
  if (!current) {
    return dir->dir.fchild;
  }
  if (current->nsib) {
    ++current->nsib->refcount;
  }
  return current->nsib;
}
