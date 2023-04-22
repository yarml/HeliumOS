#ifndef HELIUM_MODLD_H
#define HELIUM_MODLD_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <elf.h>

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
int section_whitelisted(char const *name);
Elf64_Shdr *getsh(Elf64_Ehdr *eh, Elf64_Word ndx);
Elf64_Sym *getsym(Elf64_Ehdr *eh, Elf64_Shdr *symsh, Elf64_Word ndx);
Elf64_Rela *getrela(Elf64_Ehdr *eh, Elf64_Shdr *relash, Elf64_Word ndx);

// Elf Builder
typedef struct ELF_CTX elf_ctx;

struct ELF_CTX
{

};

elf_ctx *elf_ctx_create();
void elf_ctx_destroy(elf_ctx *ctx);

#endif