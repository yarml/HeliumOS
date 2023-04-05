#include <errno.h>
#include <fs.h>

int fs_check_fcap(fsnode *file, int cap)
{
  int fsu = (file->fs->file_cap & FSCAP_USED) != 0;
  int fsc = (file->fs->file_cap & cap) != 0;

  int fu = (file->file.cap & FSCAP_USED) != 0;
  int fc = (file->file.cap & cap) != 0;

  return (
    (fsc & fc) | (~fsu & fu & fc) | (fsu & fsc) | (fsc & ~fu)
  ) & 1;
}

size_t fs_read(fsnode *file, size_t off, char *buf, size_t size)
{
  if(!file->fs->impl.fs_file_read || !fs_check_fcap(file, FSCAP_FREAD))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_read(file, off, buf, size);
}

size_t fs_tellsize(fsnode *file)
{
  if(!file->fs->impl.fs_file_tellsize || !fs_check_fcap(file, FSCAP_FTELLSIZE))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_tellsize(file);
}
