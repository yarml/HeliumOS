mmio        = 0xfffffffff8000000;
fb          = 0xfffffffffc000000;
bootboot    = 0xffffffffffe00000;
environment = 0xffffffffffe01000;
initstack   = 16K               ;

ENTRY(_start);

PHDRS {
  kernel PT_LOAD AT(0xffffffffffe02000);
}

SECTIONS {
  . = 0xffffffffffe02000;
  .text : {
    KEEP(*(.text.boot)) *(.text .text.*)
    *(.rodata .rodata.*)
    *(.data .data.*)
  } :kernel

  .bss (NOLOAD) : {
    . = ALIGN(16);
    *(.bss .bss.*)
    *(COMMON)
  } :kernel

  /DISCARD/ : { *(.eh_frame) *(.comment) }
}
