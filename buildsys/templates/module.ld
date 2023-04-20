/* Apparently, putting all symbols here is what works */
<text-ksym>
<data-ksym>
<abs-ksym>
<unk-ksym>
<bss-ksym>
<com-ksym>

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
  . = 0xffffffff80000000 + SIZEOF_HEADERS;

  .interp : {
    /* Makes interpreter equal to 'kmod' */
    BYTE(0x6B)
    BYTE(0x6D)
    BYTE(0x6F)
    BYTE(0x64)
    BYTE(0x00)
  } :text :interp

  .text : {
    *(.text .text.*)
  } :text

  .rodata : {
    <rodata-ksym>
    . = ALIGN(16);
    *(.rodata .rodata.*)
  } :text

  .rela.dyn : {
    *(.rela.*)
  } :text

  .dynamic : {
    *(.dynamic)
  } :text :dyn

  . = ALIGN(4K);

  .data : {
    *(.data .data.*)
  } :data

  .bss (NOLOAD) : {
    . = ALIGN(16);
    *(.bss .bss.*)
    *(COMMON)
  } :data

  /DISCARD/ : { *(.eh_frame) *(.comment) *(.interp) }
}
