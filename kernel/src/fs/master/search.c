#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fs.h>

#include "internal_fs.h"

fsnode *fs_resolve(char *fsname, char *names, size_t depth)
{
  filesys *fs;
  fsnode *target;

  // Search for the filesystem
  filesys_llnode *cfsn = i_fs_head;

  while(cfsn)
  {
    if(!strcmp(cfsn->fs.name, fsname))
      break;
    cfsn = cfsn->next;
  }

  // File system not found
  if(!cfsn)
    return 0;

  fs = &cfsn->fs;
  target = fs->root;

  // Now look for node in the filesystem
  for(size_t i = 0; i < depth; ++i)
  {

    // target cannot be file if we still have a name to resolve
    if(target->type != FSNODE_DIR)
      return 0;

    // target is a dir, search its subnodes for the current name to resolve
    size_t nameoff = i * FSNODE_NAMELEN;

    fsnode *starget = target->dir.fchild;

    while(starget)
    {
      if(!strcmp(starget->name, names+nameoff))
        break;
      starget = starget->nsib;
    }

    // No subnode of target had the name to resolve
    if(!starget)
      return 0;

    // Move to the next
    target = starget;

    // If current target is a link, resolve the link
    while(target->type == FSNODE_LINK)
      target = target->link.target;
  }

  return target;
}

fsnode *fs_search(char *path)
{
  char fsname[FS_NAMELEN];
  char *names = 0;
  size_t depth = 0;

  fs_pathtok(path, fsname, &names, &depth);

  fsnode *target = fs_resolve(fsname, names, depth);

  free(names);

  return target;
}

fsnode *fs_dirof(char *path)
{
    char fsname[FS_NAMELEN];
  char *names = 0;
  size_t depth = 0;

  fs_pathtok(path, fsname, &names, &depth);

  // root dir does not support dirof()
  if(!depth)
  {
    free(names);
    return 0;
  }

  fsnode *target = fs_resolve(fsname, names, depth-1);

  free(names);

  return target;
}