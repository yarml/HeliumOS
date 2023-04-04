#include <fs.h>

#include <stdio.h>

int fs_check_fcap(fsnode *node, int cap)
{
  int fsu = (node->fs->file_cap & FSCAP_USED) != 0;
  int fsc = (node->fs->file_cap & cap) != 0;

  int fu = (node->file.cap & FSCAP_USED) != 0;
  int fc = (node->file.cap & cap) != 0;

  return (
    (fsc & fc) | (~fsu & fu & fc) | (fsu & fsc) | (fsc & ~fu)
  ) & 1;
}

size_t fs_read(fsnode *file, size_t off, char *buf, size_t size)
{
  if(!file->fs->impl.fs_file_read || !fs_check_fcap(file, FSCAP_FREAD))
    return 0;

  return file->fs->impl.fs_file_read(file, off, buf, size);
}

size_t fs_tellsize(fsnode *file)
{
  if(!file->fs->impl.fs_file_tellsize || !fs_check_fcap(file, FSCAP_FTELLSIZE))
    return 0;

  return file->fs->impl.fs_file_tellsize(file);
}
