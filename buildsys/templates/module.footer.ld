ENTRY(module_init)

PHDRS
{
  module PT_LOAD;
}

SECTIONS
{
  .text : {
    KEEP(*(.text .text.*))
    *(.rodata .rodata.*)
    *(.data .data.*)
  } :module

  .bss (NOLOAD) : {
    . = ALIGN(16);
    *(.bss .bss.*)
    *(COMMON)
  } :module

  /DISCARD/ : { *(.eh_frame) *(.comment) }
}
