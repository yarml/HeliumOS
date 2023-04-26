#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "modld.h"

mod_ctx *mod_ctx_create()
{
  return calloc_or_exit(1, sizeof(mod_ctx));
}

void mod_ctx_destroy(mod_ctx *ctx)
{
  mod_section *csec = ctx->alloc_sections;
  while(csec)
  {
    mod_section *next = csec->next;
    free(csec->name);
    free(csec->content);
    free(csec);
    csec = next;
  }
  mod_nobits_section *cnsec = ctx->nobits_sections;
  while(cnsec)
  {
    mod_nobits_section *next = cnsec->next;
    free(cnsec->name);
    free(cnsec);
    cnsec = next;
  }
  free(ctx);
}

void mod_add_alloc_section(
  mod_ctx *ctx,
  char const *shname,
  void const *shcontent,
  size_t shsize, size_t shflags, size_t shalign
) {
  if(!shsize)
  {
    fprintf(stderr, "Empty allocatable section: '%s'.\n", shname);
    exit(1);
  }

  size_t shname_len = strlen(shname);

  mod_section *nsec = calloc_or_exit(1, sizeof(mod_section));
  nsec->name = calloc_or_exit(1, shname_len + 1);
  strcpy(nsec->name, shname);
  nsec->size = shsize;
  if(shcontent)
  {
    nsec->content = calloc_or_exit(1, nsec->size);
    memcpy(nsec->content, shcontent, nsec->size);
  }
  nsec->flags = shflags;
  nsec->align = shalign;
  if(shalign)
    ctx->alloc_size = ALIGN_UP(ctx->alloc_size, shalign);
  nsec->moffset = ctx->alloc_size;
  ctx->alloc_size += nsec->size;

  nsec->next = ctx->alloc_sections;
  ctx->alloc_sections = nsec;
  ++ctx->sections_count;
}

void mod_add_alloc_nobits(
  mod_ctx *ctx,
  char const *shname,
  size_t shsize, size_t shflags, size_t shalign
) {
  if(!shsize)
  {
    fprintf(stderr, "Empty nobits section: '%s'.\n", shname);
    exit(1);
  }

  size_t shname_len = strlen(shname);

  mod_nobits_section *nsec = calloc_or_exit(1, sizeof(mod_nobits_section));
  nsec->name = calloc_or_exit(1, shname_len + 1);
  strcpy(nsec->name, shname);
  nsec->size = shsize;
  nsec->flags = shflags;
  nsec->align = shalign;
  if(shalign)
    ctx->alloc_size = ALIGN_UP(ctx->alloc_size, shalign);
  nsec->moffset = ctx->alloc_size;
  ctx->alloc_size += nsec->size;

  nsec->next = ctx->nobits_sections;
  ctx->nobits_sections = nsec;
  ++ctx->sections_count;
}

size_t mod_section_moff(mod_ctx *ctx, char const *shname)
{
  mod_section *csec = ctx->alloc_sections;
  while(csec)
  {
    if(!strcmp(csec->name, shname))
      return csec->moffset;
    csec = csec->next;
  }
  mod_nobits_section *cnsec = ctx->nobits_sections;
  while(cnsec)
  {
    if(!strcmp(cnsec->name, shname))
      return cnsec->moffset;
    cnsec = cnsec->next;
  }

  return SIZE_MAX;
}

size_t mod_symoff(mod_ctx *ctx, char const *shname, size_t symval)
{
  size_t sec_off = mod_section_moff(ctx, shname);
  if(sec_off == SIZE_MAX)
    return SIZE_MAX;
  return sec_off + symval;
}

void *mod_section_content(mod_ctx *ctx, char const *shname)
{
  mod_section *csec = ctx->alloc_sections;
  while(csec)
  {
    if(!strcmp(csec->name, shname))
      return csec->content;
    csec = csec->next;
  }
  return 0;
}

void mod_genfile(mod_ctx *ctx, size_t entrypoint_off, FILE *f)
{
  size_t const ph_num = 1; // Only one program header.

  Elf64_Ehdr eh;
  memset(&eh, 0, sizeof(eh));

  // ELF identification
  memcpy(&eh.e_ident, ELFMAG, 4);
  eh.e_ident[EI_CLASS] = ELFCLASS64;
  eh.e_ident[EI_DATA] = ELFDATA2LSB;
  eh.e_ident[EI_VERSION] = EV_CURRENT;
  eh.e_ident[EI_OSABI] = ELFOSABI_NONE;
  eh.e_ident[EI_ABIVERSION] = 0;

  // ELF Header
  eh.e_type = ET_DYN;
  eh.e_machine = EM_X86_64;
  eh.e_version = EV_CURRENT;
  eh.e_entry = entrypoint_off;
  // Program header immediatly after ELF header
  eh.e_phoff = sizeof(eh);
  eh.e_shoff = 0; // No section header
  eh.e_ehsize = sizeof(Elf64_Ehdr);
  eh.e_phentsize = sizeof(Elf64_Phdr);
  eh.e_phnum = ph_num; // Only 1 program header
  eh.e_shentsize = 0;
  eh.e_shnum = 0;
  eh.e_shstrndx = SHN_UNDEF;

  size_t const data_off_begin = eh.e_phoff + ph_num * eh.e_phentsize;
  size_t data_off = data_off_begin;

  Elf64_Phdr lseg;
  memset(&lseg, 0, sizeof(lseg));
  lseg.p_type = PT_HELIUM_KMOD_LOADER;
  lseg.p_offset = data_off;
  lseg.p_filesz = ctx->sections_count * sizeof(elf64_kmod_loader_command);
  data_off += lseg.p_filesz; // data_off used later for other structures
  lseg.p_memsz = ctx->alloc_size; // Size to allocate for this kernel module
  lseg.p_align = 4096;

  printf("LSEG_filesz: %lx\n", lseg.p_filesz);

  fwrite(&eh, eh.e_ehsize, 1, f);
  fseek(f, eh.e_phoff, SEEK_SET);
  fwrite(&lseg, eh.e_phentsize, ph_num, f);
  fseek(f, data_off_begin, SEEK_SET);

  mod_section *casec = ctx->alloc_sections;
  size_t cmd_idx = 0;
  size_t const entsize = sizeof(elf64_kmod_loader_command);
  while(casec)
  {
    printf("Writing section: %s at %lx. Header at %lx \n", casec->name, data_off, data_off_begin + cmd_idx * entsize);
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_MAP;
    cmd.foff = data_off;
    cmd.moff = casec->moffset;
    cmd.size = casec->size;
    cmd.flags = casec->flags;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    fseek(f, data_off, SEEK_SET);
    fwrite(casec->content, casec->size, 1, f);
    data_off += cmd.size;
    ++cmd_idx;

    casec = casec->next;
  }
  mod_nobits_section *cnsec = ctx->nobits_sections;
  while(cnsec)
  {
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_ZMEM;
    cmd.moff = cnsec->moffset;
    cmd.size = cnsec->size;
    cmd.flags = cnsec->flags;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    ++cmd_idx;

    cnsec = cnsec->next;
  }
}
