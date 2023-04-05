#include <stdlib.h>
#include <string.h>
#include <fs.h>

#include "internal_fs.h"

void fs_rm(fsnode *node)
{
}

fsnode *fs_mknode(fsnode *parent, char *name)
{
  if(strlen(name) >= FSNODE_NAMELEN)
    return 0;

  fsnode *n = calloc(1, sizeof(fsnode));
  if(!n)
    return 0;

  strcpy(n->name, name);
  n->fs = parent->fs;

  n->parent = parent;

  n->nsib = parent->dir.fchild;
  if(parent->dir.fchild)
    parent->dir.fchild->psib = n;

  parent->dir.fchild = n;

  return n;
}
fsnode *fs_mkdir(fsnode *parent, char *name)
{
  fsnode *dir = fs_mknode(parent, name);
  if(!dir)
    return 0;

  dir->type = FSNODE_DIR;
  return dir;
}
fsnode *fs_mkfile(fsnode *parent, char *name)
{
  fsnode *file = fs_mknode(parent, name);
  if(!file)
    return 0;
  file->type = FSNODE_FILE;
  return file;
}
fsnode *fs_mklink(fsnode *parent, char *name, fsnode *target)
{
  fsnode *link = fs_mknode(parent, name);
  if(!link)
    return 0;
  link->type = FSNODE_LINK;

  link->link.target = target;

  if(target)
  {
    link->link.nlink = target->flink;
    if(target->flink)
      target->flink->link.plink = link;
    target->flink = link;
  }

  return link;
}

void fs_close(fsnode *node)
{
  if(node->refcount)
    --node->refcount;
  // In the future, I should check if the filesystem has a function to
  // call when refcount reaches 0 so that it can decide if it is time
  // to remove this node from memory
}
