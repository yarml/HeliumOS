#ifndef HELIUM_KMOD_H
#define HELIUM_KMOD_H

#define KMOD_NAMELEN (256)

typedef struct KMOD kmod;
struct KMOD
{
  char name[KMOD_NAMELEN];
};


#define KMOD_HEAP ((void *) 0x0) // 0xFFFFFFFF80000000
#define KMOD_HEAP_SIZE ((size_t) 0x78000000)

int ksym_loadf(char const *path);
int ksym_loadb(void *ksymf);

kmod *kmod_loadf(char const *path);
kmod *kmod_loadb(void *kmodf);
void kmod_uload(kmod *mod);


#endif