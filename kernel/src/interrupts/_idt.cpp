#include <meta/loop.hpp>

// I am putting this here because it takes ages to compile
// I put it here so I don't have to recompile it
void init_idt_entries(idt_entry* e)
{
    meta::idt_loop<256>::f(e);
}