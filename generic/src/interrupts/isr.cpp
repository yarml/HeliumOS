#include <interrupts/isr.hpp>
#include <debug/debug.hpp>
#include <dev/io.hpp>
#include <dev/pic.hpp>
#include <dev/framebuffer.hpp>
#include <utils.hpp>
#include <string.h>


// To avoid DRAP not supported we define the 
// actual isr handler separatly
// from the real isr handler
// aisr stands for 'actual isr'

static void aisr_keyboard()
{
    uint8_t scancode = inb(0x60);
    dbg << "scan code: " << (uint32_t) scancode << '\n';
    
    pic::send_eoi(irq::KEYBOARD);
}

__attribute__((interrupt))
void isr_keyboard(interrupt_frame*)
{
    aisr_keyboard();
}

__attribute__((interrupt)) 
void isr_unhandled(interrupt_frame*)
{
    dbg << "UNHANDLED INTERRUPT\n";
}

