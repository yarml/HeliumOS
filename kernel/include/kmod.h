#ifndef HELIUM_KMOD_H
#define HELIUM_KMOD_H

#include <mem.h>

#define KMOD_NAMELEN (256)

typedef struct KMOD kmod;
struct KMOD
{
  char name[KMOD_NAMELEN];
  mem_vseg vseg;
};

typedef struct ELF64_KMOD_LOADER_COMMAND elf64_kmod_loader_command;

#define CM_UNDEF (0)
#define CM_MAP   (1)
#define CM_ZMEM  (2)
#define CM_LDSYM (3)
#define CM_JTE   (4)


struct ELF64_KMOD_LOADER_COMMAND
{
  uint64_t command;
  union
  {
    struct
    {
      uint64_t foff;
      uint64_t moff;
      uint64_t size;
      uint64_t flags;
    } mem;
    struct
    {
      uint64_t symoff;
      uint64_t patchoff;
    } jte;
  };
};

#define KMOD_HEAP ((void *) 0xFFFFFFFF80000000)
#define KMOD_HEAP_SIZE ((size_t) 0x78000000)

int ksym_loadp(char const *path);
int ksym_loadf(fsnode *f);
int ksym_loadb(void *ksymf);

kmod *kmod_loadp(char const *path);
kmod *kmod_loadf(fsnode *f);
kmod *kmod_loadb(void *kmodf, char name[KMOD_NAMELEN]);
void kmod_uload(kmod *mod);


#endif