#ifndef ISR_HPP
#define ISR_HPP

#include <stdint.h>
#include <debug/debug.hpp>
#include <dev/pic.hpp>
#include <dev/framebuffer.hpp>
#include <interrupts/interrupt_frame.hpp>

void isr_keyboard( interrupt_frame*);
void isr_pit(      interrupt_frame*);

template<uint32_t i>
__attribute__((interrupt))
void isr_unhandled(interrupt_frame*)
{
    // dbg << "RECEIVED UNHANDLED INTERRUPT: " << i << '\n';
}

#endif /* ISR_HPP */