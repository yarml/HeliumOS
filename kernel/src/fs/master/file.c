#include <fs.h>

size_t fs_read(fsnode *file, size_t off, char *buf, size_t size)
{
  int fsr = file->fs->file_cap.read;
  int fsu = file->fs->file_cap.used;
  int fr = file->file.cap.read;
  int fu = file->file.cap.used;

  int readable = (fsr & fr | ~fsu & fu & fr | fsu & fsr | fsr & ~fu) & 1;

  if(!readable || !file->fs->impl.fs_file_read)
    return 0;

  return file->fs->impl.fs_file_read(file, off, buf, size);
}
