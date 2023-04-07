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

// FSCAP_FREAD
size_t fs_read(fsnode *file, size_t off, char *buf, size_t size)
{
  if(!file->fs->impl.fs_file_read || !fs_check_fcap(file, FSCAP_FREAD))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_read(file, off, buf, size);
}

// FSCAP_FPULL
size_t fs_pull(fsnode *file, char *buf, size_t size)
{
  if(!file->fs->impl.fs_file_pull || !fs_check_fcap(file, FSCAP_FPULL))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_pull(file, buf, size);
}

// FSCAP_FPULL
size_t fs_skip(fsnode *file, size_t size)
{
  if(
    (!file->fs->impl.fs_file_pull && !file->fs->impl.fs_file_skip) ||
    !fs_check_fcap(file, FSCAP_FPULL)
  ) {
    errno = EOPNOTSUPP;
    return 0;
  }

  if(!file->fs->impl.fs_file_skip)
  {
    char buf[size];
    return file->fs->impl.fs_file_pull(file, buf, size);
  }

  return file->fs->impl.fs_file_skip(file, size);
}

// FSCAP_FWRITE
size_t fs_write(fsnode *file, size_t off, char const *buf, size_t size)
{
    if(!file->fs->impl.fs_file_write || !fs_check_fcap(file, FSCAP_FWRITE))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_write(file, off, buf, size);
}

// FSCAP_FAPPEND
size_t fs_append(fsnode *file, char const *buf, size_t size)
{
  if(!file->fs->impl.fs_file_append || !fs_check_fcap(file, FSCAP_FAPPEND))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_append(file, buf, size);
}

// FSCAP_FTELLSIZE
size_t fs_tellsize(fsnode *file)
{
  if(!file->fs->impl.fs_file_tellsize || !fs_check_fcap(file, FSCAP_FTELLSIZE))
  {
    errno = EOPNOTSUPP;
    return 0;
  }

  return file->fs->impl.fs_file_tellsize(file);
}

