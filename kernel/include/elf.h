#ifndef HELIUM_ELF_H
#define HELIUM_ELF_H

#include <attributes.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ELF64_ID elf64_id;
typedef struct ELF64_FHEADER elf64_header;
typedef struct ELF64_PROG_HEADER elf64_prog_header;
typedef struct ELF64_SECT_HEADER elf64_sect_header;
typedef struct ELF64_SYM elf64_sym;
typedef struct ELF64_RELA elf64_rela;
typedef struct ELF64_DYN elf64_dyn;

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

struct ELF64_DYN
{
  uint64_t tag;
  union
  {
    uint64_t val;
    uint64_t ptr;
  };
};

char const *elf_etstr(uint16_t type);
char const *elf_ptstr(uint32_t type);
char const *elf_dtstr(uint64_t tag);
char const *elf_rtstr(uint32_t type);

// The following macros were copied from
// "Oracle Linker documentation Chapter 12 & Chaper 13"
// Check resource section in README

// ELF type
#define ET_NONE   (0)
#define ET_REL    (1)
#define ET_EXEC   (2)
#define ET_DYN    (3)
#define ET_CORE   (4)
#define ET_LOPROC (0xff00)
#define ET_HIPROC (0xffff)

// Program header types
#define PT_NULL    (0)
#define PT_LOAD    (1)
#define PT_DYNAMIC (2)
#define PT_INTERP  (3)
#define PT_NOTE    (4)
#define PT_SHLIB   (5)
#define PT_PHDR    (6)
#define PT_TLS     (7)
#define PT_LOOS    (0x60000000)
#define PT_HIOS    (0x6fffffff)
#define PT_LOPROC  (0x70000000)
#define PT_HIPROC  (0x7fffffff)

// Program header flags
#define PF_X        (0x1)
#define PF_W        (0x2)
#define PF_R        (0x4)
#define PF_MASKPROC (0xf0000000)

// Section types
#define SHT_NULL          (0x0)
#define SHT_PROGBITS      (0x1)
#define SHT_SYMTAB        (0x2)
#define SHT_STRTAB        (0x3)
#define SHT_RELA          (0x4)
#define SHT_HASH          (0x5)
#define SHT_DYNAMIC       (0x6)
#define SHT_NOTE          (0x7)
#define SHT_NOBITS        (0x8)
#define SHT_REL           (0x9)
#define SHT_SHLIB         (0x0A)
#define SHT_DYNSYM        (0x0B)
#define SHT_INIT_ARRAY    (0x0E)
#define SHT_FINI_ARRAY    (0x0F)
#define SHT_PREINIT_ARRAY (0x10)
#define SHT_GROUP         (0x11)
#define SHT_SYMTAB_SHNDX  (0x12)
#define SHT_NUM           (0x13)
#define SHT_LOOS          (0x60000000)

// Section flags
#define SHF_WRITE            (0x1)
#define SHF_ALLOC            (0x2)
#define SHF_EXECINSTR        (0x4)
#define SHF_MERGE            (0x10)
#define SHF_STRINGS          (0x20)
#define SHF_INFO_LINK        (0x40)
#define SHF_LINK_ORDER       (0x80)
#define SHF_OS_NONCONFORMING (0x100)
#define SHF_GROUP            (0x200)
#define SHF_TLS              (0x400)
#define SHF_MASKOS           (0x0FF00000)
#define SHF_MASKPROC         (0xF0000000)

// Special section indices
#define SHN_UNDEF         (0)
#define SHN_LORESERVE     (0xff00)
#define SHN_LOPROC        (0xff00)
#define SHN_BEFORE        (0xff00)
#define SHN_AFTER         (0xff01)
#define SHN_AMD64_LCOMMON (0xff02)
#define SHN_HIPROC        (0xff1f)
#define SHN_LOOS          (0xff20)
#define SHN_HIOS          (0xff3f)
#define SHN_ABS           (0xfff1)
#define SHN_COMMON        (0xfff2)
#define SHN_XINDEX        (0xffff)
#define SHN_HIRESERVE     (0xffff)

// RELA
#define ELF64_R_SYM(info)  ((info)>>32)
#define ELF64_R_TYPE(info) ((uint32_t)(info))

// Dynamic entry type
#define DT_NULL		         (0)
#define DT_NEEDED	         (1)
#define DT_PLTRELSZ	       (2)
#define DT_PLTGOT	         (3)
#define DT_HASH		         (4)
#define DT_STRTAB	         (5)
#define DT_SYMTAB	         (6)
#define DT_RELA		         (7)
#define DT_RELASZ	         (8)
#define DT_RELAENT	       (9)
#define DT_STRSZ	         (10)
#define DT_SYMENT	         (11)
#define DT_INIT		         (12)
#define DT_FINI		         (13)
#define DT_SONAME	         (14)
#define DT_RPATH	         (15)
#define DT_SYMBOLIC	       (16)
#define DT_REL	           (17)
#define DT_RELSZ	         (18)
#define DT_RELENT	         (19)
#define DT_PLTREL	         (20)
#define DT_DEBUG	         (21)
#define DT_TEXTREL	       (22)
#define DT_JMPREL 	       (23)
#define DT_BIND_NOW        (24)
#define DT_INIT_ARRAY      (25)
#define DT_FINI_ARRAY      (26)
#define DT_INIT_ARRAYSZ    (27)
#define DT_FINI_ARRAYSZ    (28)
#define DT_RUNPATH         (29)
#define DT_FLAGS           (30)
#define DT_ENCODING	       (32)
#define DT_PREINIT_ARRAY   (32)
#define DT_PREINIT_ARRAYSZ (33)
#define DT_MAXPOSTAGS      (34)
#define DT_LOOS		         (0x6000000d)
#define DT_HIOS		         (0x6ffff000)
#define DT_VALRNGLO	       (0x6ffffd00)
#define DT_CHECKSUM        (0x6ffffdf8)
#define DT_PLTPADSZ        (0x6ffffdf9)
#define DT_MOVEENT         (0x6ffffdfa)
#define DT_MOVESZ          (0x6ffffdfb)
#define DT_POSFLAG_1       (0x6ffffdfd)
#define DT_SYMINSZ         (0x6ffffdfe)
#define DT_SYMINENT        (0x6ffffdff)
#define DT_VALRNGHI	       (0x6ffffdff)
#define DT_ADDRRNGLO	     (0x6ffffe00)
#define DT_CONFIG          (0x6ffffefa)
#define DT_DEPAUDIT        (0x6ffffefb)
#define DT_AUDIT           (0x6ffffefc)
#define DT_PLTPAD          (0x6ffffefd)
#define DT_MOVETAB         (0x6ffffefe)
#define DT_SYMINFO         (0x6ffffeff)
#define DT_ADDRRNGHI	     (0x6ffffeff)
#define DT_RELACOUNT	     (0x6ffffff9)
#define DT_RELCOUNT	       (0x6ffffffa)
#define DT_FLAGS_1	       (0x6ffffffb)
#define DT_VERDEF	         (0x6ffffffc)
#define	DT_VERDEFNUM	     (0x6ffffffd)
#define DT_VERNEED	       (0x6ffffffe)
#define	DT_VERNEEDNUM	     (0x6fffffff)
#define DT_LOPROC	         (0x70000000)
#define DT_SPARC_REGISTER  (0x70000001)
#define DT_AUXILIARY       (0x7ffffffd)
#define DT_USED            (0x7ffffffe)
#define DT_FILTER          (0x7fffffff)
#define DT_HIPROC	         (0x7fffffff)

// x64 Relocation types
#define R_AMD64_NONE      (0)
#define R_AMD64_64        (1)
#define R_AMD64_PC32      (2)
#define R_AMD64_GOT32     (3)
#define R_AMD64_PLT32     (4)
#define R_AMD64_COPY      (5)
#define R_AMD64_GLOB_DAT  (6)
#define R_AMD64_JUMP_SLOT (7)
#define R_AMD64_RELATIVE  (8)
#define R_AMD64_GOTPCREL  (9)
#define R_AMD64_32        (10)
#define R_AMD64_32S       (11)
#define R_AMD64_16        (12)
#define R_AMD64_PC16      (13)
#define R_AMD64_8         (14)
#define R_AMD64_PC8       (15)
#define R_AMD64_PC64      (24)
#define R_AMD64_GOTOFF64  (25)
#define R_AMD64_GOTPC32   (26)
#define R_AMD64_SIZE32    (32)
#define R_AMD64_SIZE64    (33)

#endif