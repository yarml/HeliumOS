
<abs-ksym>

/* For lack of a better place, putting these here */
<unk-ksym>


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

    <text-ksym>

    . = ALIGN(16);
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

  .rela.plt : {
    *(.rela.plt)
    *(.rela.iplt)
  } :text

  .plt : {
    *(.plt) *(.iplt)
  } :text
  .plt.got : {
    *(.plt.got)
  } :text
  .plt.sec : {
    *(.plt.sec)
  } :text

  .dynamic : {
    *(.dynamic)
  } :text :dyn

  . = ALIGN(4K);

  .data : {
    <data-ksym>
    . = ALIGN(16);
    *(.data .data.*)
  } :data

  .bss (NOLOAD) : {
    <bss-ksym>

    /* I'm not sure this goes here */
    <com-ksym>

    . = ALIGN(16);
    *(.bss .bss.*)
    *(COMMON)
  } :data


  /DISCARD/ : { *(.eh_frame) *(.comment) *(.interp) }
}
