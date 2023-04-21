#include <elf.h>

char const *elf_etstr(uint16_t type)
{
  switch(type)
  {
    case ET_NONE:
      return "ET_NONE";
    case ET_REL:
      return "ET_REL";
    case ET_EXEC:
      return "ET_EXEC";
    case ET_DYN:
      return "ET_DYN";
    case ET_CORE:
      return "ET_CORE";
    default:
      return "ET_INVALID";
  }
}

char const *elf_ptstr(uint32_t type)
{
  switch(type)
  {
    case PT_NULL:
      return "PT_NULL";
    case PT_LOAD:
      return "PT_LOAD";
    case PT_DYNAMIC:
      return "PT_DYNAMIC";
    case PT_INTERP:
      return "PT_INTERP";
    case PT_NOTE:
      return "PT_NOTE";
    case PT_SHLIB:
      return "PT_SHLIB";
    case PT_PHDR:
      return "PT_PHDR";
    case PT_TLS:
      return "PT_TLS";
    default:
      return "PT_INVALID";
  }
}

char const *elf_dtstr(uint64_t tag)
{
  switch(tag)
  {
    case DT_NULL:
      return "DT_NULL";
    case DT_NEEDED:
      return "DT_NEEDED";
    case DT_PLTRELSZ:
      return "DT_PLTRELSZ";
    case DT_PLTGOT:
      return "DT_PLTGOT";
    case DT_HASH:
      return "DT_HASH";
    case DT_STRTAB:
      return "DT_STRTAB";
    case DT_SYMTAB:
      return "DT_SYMTAB";
    case DT_RELA:
      return "DT_RELA";
    case DT_RELASZ:
      return "DT_RELASZ";
    case DT_RELAENT:
      return "DT_RELAENT";
    case DT_STRSZ:
      return "DT_STRSZ";
    case DT_SYMENT:
      return "DT_SYMENT";
    case DT_INIT:
      return "DT_INIT";
    case DT_FINI:
      return "DT_FINI";
    case DT_SONAME:
      return "DT_SONAME";
    case DT_RPATH:
      return "DT_RPATH";
    case DT_SYMBOLIC:
      return "DT_SYMBOLIC";
    case DT_REL:
      return "DT_REL";
    case DT_RELSZ:
      return "DT_RELSZ";
    case DT_RELENT:
      return "DT_RELENT";
    case DT_PLTREL:
      return "DT_PLTREL";
    case DT_DEBUG:
      return "DT_DEBUG";
    case DT_TEXTREL:
      return "DT_TEXTREL";
    case DT_JMPREL:
      return "DT_JMPREL";
    case DT_BIND_NOW:
      return "DT_BIND_NOW";
    case DT_INIT_ARRAY:
      return "DT_INIT_ARRAY";
    case DT_FINI_ARRAY:
      return "DT_FINI_ARRAY";
    case DT_INIT_ARRAYSZ:
      return "DT_INIT_ARRAYSZ";
    case DT_FINI_ARRAYSZ:
      return "DT_FINI_ARRAYSZ";
    case DT_RUNPATH:
      return "DT_RUNPATH";
    case DT_FLAGS:
      return "DT_FLAGS";
    // case DT_ENCODING:
    //   return "DT_ENCODING";
    case DT_PREINIT_ARRAY:
      return "DT_PREINIT_ARRAY";
    case DT_PREINIT_ARRAYSZ:
      return "DT_PREINIT_ARRAYSZ";
    case DT_MAXPOSTAGS:
      return "DT_MAXPOSTAGS";
    case DT_CHECKSUM:
      return "DT_CHECKSUM";
    case DT_PLTPADSZ:
      return "DT_PLTPADSZ";
    case DT_MOVEENT:
      return "DT_MOVEENT";
    case DT_MOVESZ:
      return "DT_MOVESZ";
    case DT_POSFLAG_1:
      return "DT_POSFLAG_1";
    case DT_SYMINSZ:
      return "DT_SYMINSZ";
    case DT_SYMINENT:
      return "DT_SYMINENT";
    case DT_CONFIG:
      return "DT_CONFIG";
    case DT_DEPAUDIT:
      return "DT_DEPAUDIT";
    case DT_AUDIT:
      return "DT_AUDIT";
    case DT_PLTPAD:
      return "DT_PLTPAD";
    case DT_MOVETAB:
      return "DT_MOVETAB";
    case DT_SYMINFO:
      return "DT_SYMINFO";
    case DT_RELACOUNT:
      return "DT_RELACOUNT";
    case DT_RELCOUNT:
      return "DT_RELCOUNT";
    case DT_FLAGS_1:
      return "DT_FLAGS_1";
    case DT_VERDEF:
      return "DT_VERDEF";
    case DT_VERDEFNUM:
      return "DT_VERDEFNUM";
    case DT_VERNEED:
      return "DT_VERNEED";
    case DT_VERNEEDNUM:
      return "DT_VERNEEDNUM";
    case DT_SPARC_REGISTER:
      return "DT_SPARC_REGISTER";
    case DT_AUXILIARY:
      return "DT_AUXILIARY";
    case DT_USED:
      return "DT_USED";
    case DT_FILTER:
      return "DT_FILTER";
    default:
      return "DT_INVALID";
  }
}

char const *elf_rtstr(uint32_t type)
{
  switch(type)
  {
    case R_AMD64_NONE:
      return "R_AMD64_NONE";
    case R_AMD64_64:
      return "R_AMD64_64";
    case R_AMD64_PC32:
      return "R_AMD64_PC32";
    case R_AMD64_GOT32:
      return "R_AMD64_GOT32";
    case R_AMD64_PLT32:
      return "R_AMD64_PLT32";
    case R_AMD64_COPY:
      return "R_AMD64_COPY";
    case R_AMD64_GLOB_DAT:
      return "R_AMD64_GLOB_DAT";
    case R_AMD64_JUMP_SLOT:
      return "R_AMD64_JUMP_SLOT";
    case R_AMD64_RELATIVE:
      return "R_AMD64_RELATIVE";
    case R_AMD64_GOTPCREL:
      return "R_AMD64_GOTPCREL";
    case R_AMD64_32:
      return "R_AMD64_32";
    case R_AMD64_32S:
      return "R_AMD64_32S";
    case R_AMD64_16:
      return "R_AMD64_16";
    case R_AMD64_PC16:
      return "R_AMD64_PC16";
    case R_AMD64_8:
      return "R_AMD64_8";
    case R_AMD64_PC8:
      return "R_AMD64_PC8";
    case R_AMD64_PC64:
      return "R_AMD64_PC64";
    case R_AMD64_GOTOFF64:
      return "R_AMD64_GOTOFF64";
    case R_AMD64_GOTPC32:
      return "R_AMD64_GOTPC32";
    case R_AMD64_SIZE32:
      return "R_AMD64_SIZE32";
    case R_AMD64_SIZE64:
      return "R_AMD64_SIZE64";
    default:
      return "R_AMD64_INVALID";
  }
}

char const *elf_shnstr(uint16_t idx)
{
  switch(idx)
  {
    case SHN_UNDEF:
      return "SHN_UNDEF";
    case SHN_AMD64_LCOMMON:
      return "SHN_AMD64_LCOMMON";
    case SHN_ABS:
      return "SHN_ABS";
    case SHN_COMMON:
      return "SHN_COMMON";
    case SHN_XINDEX:
      return "SHN_XINDEX";
    default:
      return 0;
  }
}

char const *elf_shtstr(uint32_t type)
{
  switch(type)
  {
    case SHT_NULL:
      return "SHT_NULL";
    case SHT_PROGBITS:
      return "SHT_PROGBITS";
    case SHT_SYMTAB:
      return "SHT_SYMTAB";
    case SHT_STRTAB:
      return "SHT_STRTAB";
    case SHT_RELA:
      return "SHT_RELA";
    case SHT_HASH:
      return "SHT_HASH";
    case SHT_DYNAMIC:
      return "SHT_DYNAMIC";
    case SHT_NOTE:
      return "SHT_NOTE";
    case SHT_NOBITS:
      return "SHT_NOBITS";
    case SHT_REL:
      return "SHT_REL";
    case SHT_SHLIB:
      return "SHT_SHLIB";
    case SHT_DYNSYM:
      return "SHT_DYNSYM";
    case SHT_INIT_ARRAY:
      return "SHT_INIT_ARRAY";
    case SHT_FINI_ARRAY:
      return "SHT_FINI_ARRAY";
    case SHT_PREINIT_ARRAY:
      return "SHT_PREINIT_ARRAY";
    case SHT_GROUP:
      return "SHT_GROUP";
    case SHT_SYMTAB_SHNDX:
      return "SHT_SYMTAB_SHNDX";
    case SHT_NUM:
      return "SHT_NUM";
    default:
      return "SHT_INVALID";
  }
}
