void io_wait(void)
{
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}