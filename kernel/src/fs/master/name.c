#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fs.h>

int fs_valid_sys_name(char *name)
{
  size_t name_len = strlen(name);

  if(!name_len || name_len >= FS_NAMELEN)
    return 0;

  // Cannot start or end with '-' or '_'
  if(!isalnum(name[0]))
    return 0;
  if(!isalnum(name[name_len - 1]))
    return 0;

  for(size_t i = 1; i < name_len - 1; ++i)
    if(!isalnum(name[i]) && name[i] != '-' && name[i] != '_')
      return 0;
  return 1;
}

int fs_valid_node_name(char *name)
{
  size_t name_len = strlen(name);

  if(!name_len || name_len >= FSNODE_NAMELEN)
    return 0;

  // Cannot being with '-$%#> '
  switch(name[0])
  {
    case '-':
    case '$':
    case '%':
    case '#':
    case '>':
    case ' ':
      return 0;
  }
  // Cannot end with '- '
  switch(name[name_len - 1])
  {
    case ' ':
    case '-':
      return 0;
  }

  // We still check the first and last char for the general rules
  for(size_t i = 0; i < name_len; ++i)
  {
    char c = name[i];
    // All names should be printable
    if(!isprint(c))
      return 0;
    // Even within prints, there are more restrictions
    switch(c)
    {
      case ':': // Used in a path to denote filesystem
      case '/': // Used in a path to separate nodes
      case ';': // Used to separate list of paths
      case '~': // Ig i want to reserve this for a home-like directory
      case '\\': // No confusion
        return 0;
    }
  }

  return 1;
}

int fs_valid_path(char *path)
{
  char lpath[strlen(path)+1];
  strcpy(lpath, path);

  size_t head = 0;
  while(isspace(lpath[head]))
    ++head;

  size_t fsname_begin = head;
  while(lpath[head] && lpath[head] != ':' && !isspace(lpath[head]))
    ++head;

  char s = lpath[head];
  lpath[head] = 0;
  if(!fs_valid_sys_name(lpath+fsname_begin))
    return 0;
  lpath[head] = s;

  while(isspace(lpath[head]))
    ++head;

  if(lpath[head] != ':' || lpath[head+1] != '/' || lpath[head+2] != '/')
    return 0;
  head += 3;

  while(lpath[head])
  {
    while(isspace(lpath[head]) || lpath[head] == '/')
      ++head;
    if(!lpath[head])
      return 1;
    size_t name_begin = head;
    while(lpath[head] && lpath[head] != '/')
      ++head;
    size_t next_slash = head;
    // Backtrack to the previous non space
    while(isspace(lpath[head-1]))
      --head;

    s = lpath[head];
    lpath[head] = 0;
    if(!fs_valid_node_name(lpath+name_begin))
      return 0;
    lpath[head] = s;
    head = next_slash;
  }

  return 1;
}

// Warning; Functions assumes opath is at least as big as path
// Also assumes path is valid even if non-canonical
void fs_makecanonical(char *path, char *opath)
{
  size_t head = 0;
  size_t ohead = 0;

  while(isspace(path[head]))
    ++head;

  while(path[head] && path[head] != ':' && !isspace(path[head]))
  {
    opath[ohead] = path[head];
    ++ohead;
    ++head;
  }

  // Skip all whitespaces after filesystem name
  while(isspace(path[head]))
    ++head;

  // Now path[head] = ':'
  opath[ohead++] = ':';
  opath[ohead++] = '/';
  opath[ohead++] = '/';
  head += 3;

  // Now path[head] is the second / of the path
  while(path[head])
  {
    // skip all whitespaces
    while(isspace(path[head]))
      ++head;

    // If we find a '/' after the whitespace, then add a '/' to output, and skip
    // all following '/' and spaces until first word or end of path
    if(path[head] == '/')
    {
      if(opath[ohead-1] != '/')
      {
        opath[ohead] = '/';
        ++ohead;
      }
      ++head;
      while(path[head] == '/' || isspace(path[head]))
        ++head;
    }

    size_t wb_head = head;
    size_t wb_ohead = ohead;
    while(path[head] && path[head] != '/')
    {
      opath[ohead] = path[head];
      ++ohead;
      ++head;
    }
    // track back the last sequence of spaces
    while(isspace(opath[ohead-1]))
      --ohead;

    // If this was a '..', then we roll back to
    // just after the second previous '/'
    if(head - wb_head == 2 && path[wb_head] == '.' && path[wb_head] == '.')
    {
      ohead -= 4;
      while(opath[ohead-1] != '/')
        --ohead;
    }

    // If this was a '.'
    if(head - wb_head == 1 && path[wb_head] == '.')
      ohead = wb_ohead;
  }
  opath[ohead] = 0;
}

void fs_basename(char *path, char *name)
{
  char *names = 0;
  size_t n = 0;
  char fsname[FS_NAMELEN];
  fs_pathtok(path, fsname, &names, &n);

  if(!n)
  {
    name[0] = 0;
    // errno is set by fs_pathtok
    return;
  }

  size_t nname_off = (n - 1) * FSNODE_NAMELEN;
  strcpy(name, names+nname_off);
}

// Assumes path is valid
// *nodes needs to be free()d later
void fs_pathtok(char *path, char *fsname, char **nodes, size_t *len)
{
  size_t path_len = strlen(path);
  char canon_path[path_len+1];

  fs_makecanonical(path, canon_path);

  size_t canon_path_len = strlen(canon_path);
  *len = 0;
  for(size_t i = 0; i < canon_path_len; ++i)
  {
    if(canon_path[i] == '/')
      ++*len;
  }

  --*len;
  if(canon_path[canon_path_len-1] == '/')
    --*len;

  size_t head = 0;
  while(canon_path[head] != ':')
  {
    fsname[head] = canon_path[head];
    ++head;
  }
  fsname[head] = 0;

  head += 3;

  char *nbuf = calloc(*len, FSNODE_NAMELEN);
  if(!nbuf)
  {
    *len = 0;
    // errno set by calloc
    return;
  }

  *nodes = nbuf;

  for(size_t i = 0; i < *len; ++i)
  {
    size_t ohead = i * FSNODE_NAMELEN;
    while(canon_path[head] && canon_path[head] != '/')
    {
      nbuf[ohead] = canon_path[head];
      ++ohead;
      ++head;
    }
    nbuf[ohead] = 0;
    ++ohead;
    ++head;
  }
}

static void fsnode_print_rscv(fsnode *node, size_t level)
{
  if(!node)
    return;

  for(size_t i = 0; i < level; ++i)
    printd(" ");

  printd("%s", node->name);

  switch(node->type)
  {
    case FSNODE_DIR:
      printd("/\n");
      fsnode_print_rscv(node->dir.fchild, level+1);
      break;
    case FSNODE_LINK:
      printd(" ->");
      if(node->link.target)
        printd("%s\n", node->link.target->name);
      else
        printd("-\n");
      break;
    case FSNODE_FILE:
      printd("\n");
      break;
  }

  fsnode_print_rscv(node->nsib, level);
}

void fs_print(filesys *fs)
{
  printd("%s://\n", fs->name);
  fsnode_print_rscv(fs->root->dir.fchild, 1);
}
