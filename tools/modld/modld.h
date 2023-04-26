#ifndef HELIUM_MODLD_H
#define HELIUM_MODLD_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <elf.h>

#define PT_HELIUM_KMOD_LOADER (0x60000000)

// Taken from helium source at: /kernel/include/utils.h
// b should be a power of 2
#define ALIGN_DN(n, b) (((n)          ) & ~((b) - 1))
#define ALIGN_UP(n, b) (((n) + (b) - 1) & ~((b) - 1))

#define EMPTY_STR ""

// Misc
FILE *fopen_or_exit(char const *path, char const *mode);
void *calloc_or_exit(size_t n, size_t size);
uint64_t elf_hash(char const *name);

// Elf Parser
void elf_parser_init();
Elf64_Ehdr *verify_ehdr(uint8_t *frel_data, size_t frel_size);
Elf64_Shdr *verify_shdr(Elf64_Ehdr *eh, Elf64_Word ndx);
char const *getstrtab(Elf64_Ehdr *eh);
char const *get_shstr(Elf64_Ehdr *eh, Elf64_Word name);
Elf64_Shdr *getsymtab(Elf64_Ehdr *eh);
int section_whitelisted(char const *name);
Elf64_Shdr *getsh(Elf64_Ehdr *eh, Elf64_Word ndx);
char const *getshname(Elf64_Ehdr *eh, Elf64_Word ndx);
Elf64_Sym *getsym(Elf64_Ehdr *eh, Elf64_Shdr *symsh, Elf64_Word ndx);
Elf64_Sym *getsymn(Elf64_Ehdr *eh, char const *name);
Elf64_Rela *getrela(Elf64_Ehdr *eh, Elf64_Shdr *relash, Elf64_Word ndx);
void *get_shcontent(Elf64_Ehdr *eh, Elf64_Shdr *sh);
int sh_isalloc(Elf64_Shdr *sh);

// Mod file Builder
typedef struct MOD_CTX mod_ctx;
typedef struct MOD_SECTION mod_section;
typedef struct MOD_NOBITS_SECTION mod_nobits_section;

struct MOD_CTX
{
  size_t alloc_size;
  size_t sections_count; // Count of alloc and nobits sections
  mod_section *alloc_sections;
  mod_nobits_section *nobits_sections;
};

struct MOD_SECTION
{
  char *name;
  void *content;
  size_t size;
  size_t moffset;
  size_t align;
  size_t flags;

  mod_section *next;
};

struct MOD_NOBITS_SECTION
{
  char *name;
  size_t size;
  size_t moffset;
  size_t align;
  size_t flags;

  mod_nobits_section *next;
};

typedef struct ELF64_KMOD_LOADER_COMMAND elf64_kmod_loader_command;

#define CM_UNDEF (0)
#define CM_MAP   (1)
#define CM_ZMEM  (2)

struct ELF64_KMOD_LOADER_COMMAND
{
  uint64_t command;
  uint64_t foff;
  uint64_t moff;
  uint64_t size;
  uint64_t flags;
};

mod_ctx *mod_ctx_create();
void mod_ctx_destroy(mod_ctx *ctx);

void mod_add_alloc_section(
  mod_ctx *ctx,
  char const *shname,
  void const *shcontent,
  size_t shsize, size_t shflags, size_t shalign
);
void mod_add_alloc_nobits(
  mod_ctx *ctx,
  char const *shname,
  size_t size, size_t shflags, size_t shalign
);

void *mod_section_content(mod_ctx *ctx, char const *shname);

size_t mod_section_moff(mod_ctx *ctx, char const *shname);
size_t mod_symoff(mod_ctx *ctx, char const *shname, size_t symval);
void mod_genfile(mod_ctx *ctx, size_t entrypoint_off, FILE *f);

#endif