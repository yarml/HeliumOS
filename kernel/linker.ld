ENTRY(_start)

SECTIONS
{

    . = 0xC0100000;

    kernel_virtual_start = .;
    kernel_physical_start = . - 0xC0000000;

    .text ALIGN(0x1000) : AT(ADDR(.text)-0xC0000000)
    {
        *(multiboot)
        *(.text)
        *(.gnu.linkonce.t*)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata)-0xC0000000)
    {
        start_ctors = .;
        *(SORT(.ctors*))
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data)-0xC0000000)
    {
        *(.data)
        *(.gnu.linkonce.d*)
    }

    .bss ALIGN(0x1000): AT(ADDR(.bss)-0xC0000000)
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
        ebss = .;
    }

    kernel_virtual_end = .;
    kernel_physical_end = . - 0xC0000000;
}