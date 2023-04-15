ENTRY(module_init)

PHDRS
{
  headers PT_PHDR PHDRS;
  interp PT_INTERP;
  text PT_LOAD FILEHDR PHDRS;
  data PT_LOAD;
  dyn PT_DYNAMIC;
}

SECTIONS
{
  . = SIZEOF_HEADERS;

  .interp : {
    /* Makes interpreter equal to 'kmod' */
    BYTE(0x6B)
    BYTE(0x6D)
    BYTE(0x6F)
    BYTE(0x64)
    BYTE(0x00)
  } :text :interp

  .text : {
    . = ALIGN(16);
    *(.text .text.*)
  } :text

  .rodata : {
    . = ALIGN(16);
    *(.rodata .rodata.*)
  } :text

  . = ALIGN(4K);

  .data : {
    . = ALIGN(16);
    *(.data .data.*)
  } :data

  .bss (NOLOAD) : {
    . = ALIGN(16);
    *(.bss .bss.*)
    *(COMMON)
  } :data

  .dynamic : {
    *(.dynamic)
  } :data :dyn

  /DISCARD/ : { *(.eh_frame) *(.comment) *(.interp) }
}
