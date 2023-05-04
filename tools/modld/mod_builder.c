#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "modld.h"

mod_ctx *mod_ctx_create()
{
  mod_ctx *ctx = calloc_or_exit(1, sizeof(mod_ctx));
  ctx->symtab = calloc_or_exit(1, 2);
  ctx->symtab_size = 2;
  return ctx;
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
  mod_jte *cjte = ctx->jt_entries;
  while(cjte)
  {
    mod_jte *next = cjte->next;
    free(cjte->refs);
    free(cjte);
    cjte = next;
  }
  mod_gote *cgote = ctx->got_entries;
  while(cgote)
  {
    mod_gote *next = cgote->next;
    free(cgote->refs);
    free(cgote);
    cgote = next;
  }
  free(ctx->symtab);
  free(ctx);
}

size_t mod_addsym(mod_ctx *ctx, char const *sym)
{
  size_t sym_len = strlen(sym);
  if(!sym_len)
    return 0;
  size_t exists_off = mod_searchsym(ctx, sym);
  if(exists_off)
    return exists_off;
  char *new_symtab = realloc(ctx->symtab, ctx->symtab_size + sym_len + 1);
  if(!new_symtab)
  {
    fprintf(stderr, "Could not allocate space for symbol table\n");
    exit(1);
  }
  ctx->symtab = new_symtab;
  size_t off = ctx->symtab_size;
  strcpy(ctx->symtab + off, sym);
  ctx->symtab_size += sym_len + 1;
  return off;
}

size_t mod_searchsym(mod_ctx *ctx, char const *sym)
{
  for(size_t i = 0; i < ctx->symtab_size; ++i)
  {
    if(!strcmp(ctx->symtab + i, sym))
      return i;
    i += strlen(ctx->symtab + i);
  }
  return 0;
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

mod_section *mod_search_alloc_section(mod_ctx *ctx, char const *shname)
{
  mod_section *csec = ctx->alloc_sections;
  while(csec)
  {
    if(!strcmp(csec->name, shname))
      return csec;
    csec = csec->next;
  }
  return 0;
}

mod_jte *mod_add_jte(mod_ctx *ctx, char const *symname)
{
  size_t nameoff = mod_addsym(ctx, symname);
  mod_jte *jte = calloc_or_exit(1, sizeof(mod_jte));
  jte->nameoff = nameoff;
  jte->index = ctx->jte_count;
  jte->next = ctx->jt_entries;
  ctx->jt_entries = jte;
  ++ctx->jte_count;
  return jte;
}

void mod_refjte(
  mod_ctx *ctx,
  char const *symname,
  char const *shname,
  size_t off
) {
  mod_jte *jte = 0;
  mod_jte *cjte = ctx->jt_entries;
  while(cjte)
  {
    if(!strcmp(ctx->symtab + cjte->nameoff, symname))
    {
      jte = cjte;
      break;
    }
    cjte = cjte->next;
  }
  if(!jte)
    jte = mod_add_jte(ctx, symname);
  mod_jtref_patch *newrefs = reallocarray(
    jte->refs,
    jte->refcount + 1,
    sizeof(mod_jtref_patch)
  );
  if(!newrefs)
  {
    fprintf(
      stderr,
      "Could not allocate memory for jump table entry references"
    );
    exit(1);
  }
  mod_section *tsection = mod_search_alloc_section(ctx, shname);
  if(!tsection)
  {
    fprintf(stderr, "Relocation in an unallocated section\n");
    exit(1);
  }
  jte->refs = newrefs;
  jte->refs[jte->refcount].tsection = tsection;
  jte->refs[jte->refcount].offset = off;
  ++jte->refcount;
  ++ctx->jte_refcount;
}

void mod_refgote(
  mod_ctx *ctx,
  char const *shname,
  size_t patchoff,
  size_t symval
) {
  mod_gote *gote = 0;
  // First search if a symbol with the same offset has a GOT entry
  mod_gote *cgote = ctx->got_entries;
  while(cgote)
  {
    if(cgote->symval == symval)
    {
      gote = cgote;
      break;
    }
    cgote = cgote->next;
  }

  // If we didn't find any GOT entry with the same symboll offset,
  // we should create a new one
  if(!gote)
  {
    gote = calloc_or_exit(1, sizeof(mod_gote));
    gote->index = ctx->gote_count;
    gote->next = ctx->got_entries;
    gote->symval = symval;

    ctx->got_entries = gote;
    ++ctx->gote_count;
  }

  mod_gotref_patch *newrefs = reallocarray(
    gote->refs,
    gote->refcount + 1,
    sizeof(mod_gotref_patch)
  );
  if(!newrefs)
  {
    fprintf(
      stderr,
      "Could not allocate memory for GOT entry references"
    );
    exit(1);
  }
  mod_section *tsection = mod_search_alloc_section(ctx, shname);
  if(!tsection)
  {
    fprintf(stderr, "Relocation in an unallocated section\n");
    exit(1);
  }

  gote->refs = newrefs;
  gote->refs[gote->refcount].tsection = tsection;
  gote->refs[gote->refcount].offset = patchoff;
  ++gote->refcount;
  ++ctx->got_refcount;
}

void mod_refkgote(
  mod_ctx *ctx,
  char const *shname,
  size_t patchoff,
  char const *name
) {
  size_t symoff = mod_addsym(ctx, name);

  // Check if there is a KGOTE that already has this symbol
  mod_kgote *kgote = 0;
  mod_kgote *ckgote = ctx->kgot_entries;
  while(ckgote)
  {
    if(ckgote->nameoff == symoff)
    {
      kgote = ckgote;
      break;
    }
    ckgote = ckgote->next;
  }

  if(!kgote)
  {
    kgote = calloc_or_exit(1, sizeof(*kgote));
    kgote->nameoff = symoff;
    kgote->next = ctx->kgot_entries;
    ctx->kgot_entries = kgote;
    kgote->index = ctx->gote_count;

    ++ctx->gote_count;
  }

  mod_gotref_patch *newrefs = reallocarray(
    kgote->refs,
    kgote->refcount + 1,
    sizeof(mod_gotref_patch)
  );
  if(!newrefs)
  {
    fprintf(
      stderr,
      "Could not allocate memory for Kernel GOT entry references"
    );
    exit(1);
  }
  mod_section *tsection = mod_search_alloc_section(ctx, shname);
  if(!tsection)
  {
    fprintf(stderr, "Relocation in an unallocated section\n");
    exit(1);
  }

  kgote->refs = newrefs;
  kgote->refs[kgote->refcount].tsection = tsection;
  kgote->refs[kgote->refcount].offset = patchoff;
  ++kgote->refcount;
  ++ctx->got_refcount;
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
  size_t jt_moff = ctx->alloc_size;
  ctx->alloc_size += ctx->jte_count * 5;

  // GOT needs to be aligned to 8 bytes
  if(ctx->gote_count)
    ctx->alloc_size = ALIGN_UP(ctx->alloc_size, 8);

  size_t got_moff = ctx->alloc_size;
  ctx->alloc_size += ctx->gote_count * sizeof(void *); // * 8

  // Apply JT patches
  mod_jte *cjte = ctx->jt_entries;
  while(cjte)
  {
    for(size_t i = 0; i < cjte->refcount; ++i)
    {
      mod_section *ts = cjte->refs[i].tsection;
      size_t off = cjte->refs[i].offset;
      uint32_t patch = jt_moff + 5 * cjte->index - ts->moffset - off - 4;
      memcpy(ts->content + off, &patch, 4);
    }
    cjte = cjte->next;
  }

  // Apply GOT patches
  mod_gote *cgote = ctx->got_entries;
  while(cgote)
  {
    for(size_t i = 0; i < cgote->refcount; ++i)
    {
      mod_section *ts = cgote->refs[i].tsection;
      size_t off = cgote->refs[i].offset;
      uint32_t patch =
        got_moff + sizeof(void *) * cgote->index - ts->moffset - off - 4;
      memcpy(ts->content + off, &patch, 4);
    }
    cgote = cgote->next;
  }
  // Apply KGOT patches
  mod_kgote *ckgote = ctx->kgot_entries;
  while(ckgote)
  {
    for(size_t i = 0; i < ckgote->refcount; ++i)
    {
      mod_section *ts = ckgote->refs[i].tsection;
      size_t off = ckgote->refs[i].offset;
      uint32_t patch =
        got_moff + sizeof(void *) * ckgote->index - ts->moffset - off - 4;
      memcpy(ts->content + off, &patch, 4);
    }
    ckgote = ckgote->next;
  }


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

  // Number of loader commands:
  // A command to load symbol table (LDSYM)
  // There is a command for each section (MAP/ZMEM)
  // A command for each jump table entry (JTE)
  // A command for each GOT entry (ADDBASE)
  size_t cmd_count = ctx->sections_count + ctx->jte_count + ctx->gote_count + 1;

  // If we have any jump entry, then we also have an additional
  // command to map the jump table (MAP)
  if(ctx->jte_count)
    ++cmd_count;

  // If we have any GOT entry, then we also have an additional
  // command to map the GOT (MAP)
  if(ctx->gote_count)
    ++cmd_count;

  Elf64_Phdr lseg;
  memset(&lseg, 0, sizeof(lseg));
  lseg.p_type = PT_HELIUM_KMOD_LOADER;
  lseg.p_offset = data_off;
  lseg.p_filesz = cmd_count * sizeof(elf64_kmod_loader_command);
  data_off += lseg.p_filesz; // data_off used later for other structures
  lseg.p_memsz = ctx->alloc_size; // Size to allocate for this kernel module
  lseg.p_align = 4096;

  // Write Ehdr & Phdr
  fwrite(&eh, eh.e_ehsize, 1, f);
  fseek(f, eh.e_phoff, SEEK_SET);
  fwrite(&lseg, eh.e_phentsize, ph_num, f);
  fseek(f, data_off_begin, SEEK_SET);

  // Write the LDSYM command
  elf64_kmod_loader_command cmd_ldsym;
  memset(&cmd_ldsym, 0, sizeof(cmd_ldsym));
  cmd_ldsym.command = CM_LDSYM;
  cmd_ldsym.mem.foff = data_off;

  fseek(f, data_off_begin, SEEK_SET);
  fwrite(&cmd_ldsym, sizeof(cmd_ldsym), 1, f);
  fseek(f, data_off, SEEK_SET);
  fwrite(ctx->symtab, ctx->symtab_size, 1, f);
  data_off += ctx->symtab_size;

  mod_section *casec = ctx->alloc_sections;
  size_t cmd_idx = 1; // CMD 0 is LDSYM
  size_t const entsize = sizeof(elf64_kmod_loader_command);
  while(casec)
  {
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_MAP;
    cmd.mem.foff = data_off;
    cmd.mem.moff = casec->moffset;
    cmd.mem.size = casec->size;
    cmd.mem.flags = casec->flags;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    fseek(f, data_off, SEEK_SET);
    fwrite(casec->content, casec->size, 1, f);
    data_off += cmd.mem.size;
    ++cmd_idx;

    casec = casec->next;
  }
  mod_nobits_section *cnsec = ctx->nobits_sections;
  while(cnsec)
  {
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_ZMEM;
    cmd.mem.moff = cnsec->moffset;
    cmd.mem.size = cnsec->size;
    cmd.mem.flags = cnsec->flags;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    ++cmd_idx;

    cnsec = cnsec->next;
  }

  if(!ctx->jte_count) // If we don't have any jtes, we can jump to GOTEs
    goto setup_got;

  // Now for the jump table
  // First we map it
  elf64_kmod_loader_command jtmap_cmd;
  memset(&jtmap_cmd, 0, sizeof(jtmap_cmd));
  jtmap_cmd.command = CM_MAP;
  jtmap_cmd.mem.foff = data_off;
  jtmap_cmd.mem.moff = jt_moff;
  jtmap_cmd.mem.size = ctx->jte_count * 5;
  jtmap_cmd.mem.flags = SHF_ALLOC | SHF_EXECINSTR;

  fseek(
    f,
    data_off_begin + cmd_idx * entsize,
    SEEK_SET
  );
  fwrite(&jtmap_cmd, sizeof(jtmap_cmd), 1, f);

  // Write the Jump table
  uint8_t *jt = calloc_or_exit(ctx->jte_count, 5);
  for(size_t i = 0; i < ctx->jte_count; ++i)
  {
    jt[5*i] = 0xE9; // jmp instruction
    memset(jt+5*i+1, 0, 4);
  }
  fseek(f, data_off, SEEK_SET);
  fwrite(jt, 5, ctx->jte_count, f);
  free(jt);

  ++cmd_idx;
  data_off += ctx->jte_count * 5;

  // Now we add JTE commands so that the loader can correctly make
  // the kernel module jump to kernel functions
  cjte = ctx->jt_entries;
  while(cjte)
  {
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_JTE;
    cmd.patch.val = cjte->nameoff;
    cmd.patch.patchoff = jt_moff + 5 * cjte->index + 1;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    ++cmd_idx;
    cjte = cjte->next;
  }

  setup_got:
  if(!ctx->gote_count) // If no GOT entries, we're done
    return;

  // First we generate the command to map the GOT
  elf64_kmod_loader_command gotmap_cmd;
  memset(&gotmap_cmd, 0, sizeof(gotmap_cmd));
  gotmap_cmd.command = CM_MAP;
  gotmap_cmd.mem.foff = data_off;
  gotmap_cmd.mem.moff = got_moff;
  gotmap_cmd.mem.size = ctx->gote_count * sizeof(void *);
  gotmap_cmd.mem.flags = SHF_ALLOC;

  fseek(
    f,
    data_off_begin + cmd_idx * entsize,
    SEEK_SET
  );
  fwrite(&gotmap_cmd, sizeof(gotmap_cmd), 1, f);

  ++cmd_idx;

  // Next we write the GOT
  // Why do I use heap memory? no clue
  uint8_t *got = calloc_or_exit(ctx->gote_count, sizeof(void *));
  fseek(f, data_off, SEEK_SET);
  fwrite(got, sizeof(void *), ctx->gote_count, f);
  free(got);

  data_off += ctx->gote_count * sizeof(void *);

  // Now add the ADDBASE commands for GOT entries
  cgote = ctx->got_entries;
  while(cgote)
  {
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_ADDBASE;
    cmd.patch.patchoff = got_moff + sizeof(void *) * cgote->index;
    cmd.patch.val = cgote->symval;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    ++cmd_idx;

    cgote = cgote->next;
  }
  // Now add the KSYM commands for KGOT entries
  ckgote = ctx->kgot_entries;
  while(ckgote)
  {
    elf64_kmod_loader_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CM_KSYM;
    cmd.patch.patchoff = got_moff + sizeof(void *) * ckgote->index;
    cmd.patch.val = ckgote->nameoff;

    fseek(
      f,
      data_off_begin + cmd_idx * entsize,
      SEEK_SET
    );
    fwrite(&cmd, sizeof(cmd), 1, f);
    ++cmd_idx;

    ckgote = ckgote->next;
  }
}