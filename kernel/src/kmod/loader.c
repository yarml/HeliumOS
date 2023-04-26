#include <stdlib.h>
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
  kmod *mod = kmod_loadb(buf, f->name);
  fs_close(f);
  return mod;
}

kmod *kmod_loadb(void *kmodf, char name[KMOD_NAMELEN])
{
  tpd("### Loading kernel module ###\n");


  elf64_header *eh = kmodf;
  elf64_prog_header *ph = kmodf + eh->phoff;
  elf64_kmod_loader_command *cmd = kmodf + ph->offset;
  size_t cmd_count = ph->file_size / sizeof(elf64_kmod_loader_command);

  tpd("ALLOC size=%lz\n", ph->mem_size);
  mem_vseg kmod_vseg = mem_alloc_vblock(
    ph->mem_size,
    MAPF_W | MAPF_R | MAPF_X, // TODO: Give 0 permissions by default
    KMOD_HEAP,
    KMOD_HEAP_SIZE
  );

  if(kmod_vseg.error)
    return 0;

  kmod *mod = calloc(1, sizeof(kmod));

  strcpy(mod->name, name);
  mod->vseg = kmod_vseg;

  void *base = kmod_vseg.ptr;

  for(size_t i = 0; i < cmd_count; ++i)
  {
    switch(cmd[i].command)
    {
      case CM_UNDEF:
        tpd("UNDEF #\n");
        break;
      case CM_MAP:
        tpd(
          "MAP foff=%p, moff=%p, size=%lu, flags=%lx\n",
          cmd[i].foff, cmd[i].moff, cmd[i].size, cmd[i].flags
        );
        memcpy(base + cmd[i].moff, kmodf + cmd[i].foff, cmd[i].size);
        break;
      case CM_ZMEM:
        tpd(
          "ZMEM moff=%p, size=%lu, flags=%lx\n",
          cmd[i].moff, cmd[i].size, cmd[i].flags
        );
        memset(base + cmd[i].moff, 0, cmd[i].size);
        break;
    }
  }
  tpd("ENTRYPOINT moff=%p\n", eh->entrypoint);
  int (*mod_init)() = base + eh->entrypoint;
  tpd("Return value: %d\n", mod_init());
  return 0;
}

void kmod_uload(kmod *mod)
{
  
}
