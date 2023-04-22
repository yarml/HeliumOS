#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <utils.h>
#include <kmod.h>
#include <elf.h>
#include <mem.h>
#include <fs.h>

kmod *kmod_loadf(char const *path)
{
  fsnode *f = fs_search(path);
  if(!f)
    return 0;
  size_t fsize = fs_tellsize(f);
  if(!fsize)
  {
    fs_close(f);
    return 0;
  }
  char buf[fsize];
  size_t read = 0;
  while(read < fsize)
  {
    errno = 0;
    size_t cr = fs_read(f, read, buf + read, fsize - read);
    if(!cr && errno)
    {
      fs_close(f);
      return 0;
    }
    read += cr;
  }
  fs_close(f);
  return kmod_loadb(buf);
}

kmod *kmod_loadb(void *kmodf)
{
  tpd("### Loading kernel module ###\n");

  return 0;
}

void kmod_uload(kmod *mod)
{
  
}
