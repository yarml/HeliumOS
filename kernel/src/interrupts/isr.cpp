#include <interrupts/isr.hpp>
#include <debug/debug.hpp>
#include <dev/keyboard.hpp>
#include <dev/pic.hpp>
#include <dev/pit.hpp>

__attribute__((interrupt))
void isr_keyboard(interrupt_frame*)
{
    keyboard::keyboard_int();
    pic::send_eoi(irq::KEYBOARD);
}

__attribute__((interrupt))
void isr_pit(interrupt_frame*)
{
    pit::pit_int();
    pic::send_eoi(irq::PIT);
}
