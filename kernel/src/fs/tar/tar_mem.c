#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <fs.h>

#include <fs/tar.h>

#include "../master/internal_fs.h"

void tar_header_print(tar_header *header)
{
  printf("Name: %s%s\n", header->name_pref, header->name);
  printf("Type: %c\n", header->type);
  if(header->type == '1' || header->type == '2')
    printf("Link to: %s", header->link_name);
  printf("Size: %lu\n", tar_file_size(header));
}

size_t tar_file_size(tar_header *header)
{
  return stou(header->size, 0, 8);
}

char *tar_entry_type(tar_header *header)
{
  switch(header->type)
  {
    case TAR_ENTRY_TYPE_FILE:
      return "file";
    case TAR_ENTRY_TYPE_HLINK:
    case TAR_ENTRY_TYPE_SLINK:
      return "link";
    case TAR_ENTRY_TYPE_DIR:
      return "dir";
    default:
      return "unknown";
  }
}

// Implementation functions
static size_t tar_file_read(fsnode *f, size_t off, char *buf, size_t size)
{
  tpf("Reading %lu bytes from tar file\n", size);
  tar_header *header = f->ext;
  void *content = header + 1;

  size_t fsize = tar_file_size(header);

  if(off > fsize)
    return 0;

  size_t acsize; // actual size

  if(fsize - off < size)
    acsize = fsize - off;
  else
    acsize = size;

  tpf("Actual size is %lu\n", acsize);

  memcpy(buf, content, acsize);
  return acsize;
}
static size_t tar_file_tellsize(fsnode *file)
{
  tar_header *header = file->ext;

  return tar_file_size(header);
}

filesys *tar_mkimfs(char *fsname, void *membuf, size_t size)
{
  fsimpl tar_impl;
  memset(&tar_impl, 0, sizeof(tar_impl));

  tar_impl.fs_file_read = tar_file_read;
  tar_impl.fs_file_tellsize = tar_file_tellsize;

  filesys *fs = fs_mount(fsname);
  if(!fs)
    return 0;

  fs->file_cap = FSCAP_USED | FSCAP_FREAD | FSCAP_FTELLSIZE;

  fs->impl = tar_impl;

  size_t fsname_len = strlen(fsname);

  char *path = 0;
  size_t path_cap = 0; // NULL termination not accounted for

  tar_header *current_header = membuf;

  // FIXME: The implementation assumes that in tar files, all directories
  // come before their sub nodes, which is not necessarily true(or at least
  // I didn't find any source saying it is), mkbootimg seems to put directories
  // entries before entries of subnodes, so this works for now
  while(1)
  {
    if(
      (uintptr_t) current_header + sizeof(tar_header)
        > (uintptr_t) membuf + size ||
      memcmp(current_header->ustar, "ustar  ", 8)
    )
      break;

    size_t name1_len = strlen(current_header->name_pref);
    size_t name2_len = strlen(current_header->name);
    size_t name_len = name1_len + name2_len;


    // +3 for '://', +1 for NULL termination
    size_t path_len = fsname_len + name_len + 3;

    if(path_len >= path_cap)
    {
      path = realloc(path, path_len+1);
      path_cap = path_len+1;
    }

    snprintf(
      path, path_cap,
      "%s://%s%s",
      fsname, current_header->name_pref, current_header->name
    );

    tar_header *ch = current_header;

    current_header =
      (void *) current_header
      + ALIGN_UP(sizeof(tar_header), 512)
      + ALIGN_UP(tar_file_size(current_header), 512);

    // Duplicate entry, this probably means we got a .. or something
    if(fs_search(path))
      continue;

    fsnode *dir = fs_dirof(path);

    char node_name[FSNODE_NAMELEN];
    fs_nodename(path, node_name);

    fsnode *cf;

    switch(ch->type)
    {
      case TAR_ENTRY_TYPE_FILE:
        cf = fs_mkfile(dir, node_name);
        break;
      case TAR_ENTRY_TYPE_DIR:
        cf = fs_mkdir(dir, node_name);
        break;
      case TAR_ENTRY_TYPE_HLINK:
      case TAR_ENTRY_TYPE_SLINK:
        /* Not implemented */
        break;
    }

    cf->ext = ch;
  }
  return fs;
}
