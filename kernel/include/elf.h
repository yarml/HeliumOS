#ifndef HELIUM_ELF_H
#define HELIUM_ELF_H

#include <attributes.h>
#include <stdint.h>

typedef struct ELF64_ID elf64_id;
typedef struct ELF64_FHEADER elf64_header;
typedef struct ELF64_PROG_HEADER elf64_prog_header;
typedef struct ELF64_SECT_HEADER elf64_sect_header;
typedef struct ELF64_SYM elf64_sym;
typedef struct ELF64_RELA elf64_rela;

struct ELF64_ID
{
  uint8_t magic[4];
  uint8_t class;
  uint8_t endian;
  uint8_t version;
  uint8_t osabi;
  uint8_t abiver;
  uint8_t pad0[7];
} pack;

struct ELF64_FHEADER
{
  elf64_id id;
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint64_t entrypoint;
  uint64_t phoff;
  uint64_t shoff;
  uint32_t flags;
  uint16_t size;
  uint16_t phent_size;
  uint16_t pht_len;
  uint16_t shent_size;
  uint16_t sht_len;
  uint16_t shstridx;
} pack;

struct ELF64_PROG_HEADER
{
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vadr;
  uint64_t padr;
  uint64_t file_size;
  uint64_t mem_size;
  uint64_t align;
};

struct ELF64_SECT_HEADER
{
  uint32_t name;
  uint32_t type;
  uint64_t flags;
  uint64_t addr;
  uint64_t offset;
  uint64_t size;
  uint32_t link;
  uint32_t info;
  uint64_t addr_align;
  uint64_t ent_size;
};

struct ELF64_SYM
{
  uint32_t name;
  uint8_t info;
  uint8_t other;
  uint16_t shidx;
  uint64_t value;
  uint64_t size;
};

struct ELF64_RELA
{
  uint64_t offset;
  uint64_t info;
  int64_t addend;
};

#define SHT_SYMTAB (0x02)
#define SHT_STRTAB (0x03)
#define SHT_RELA   (0x04)

#endif