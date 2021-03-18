// If you hate meta programming then don't look here

#ifndef LOOP_HPP
#define LOOP_HPP

#include <stdint.h>
#include <memory/gdt.hpp>
#include <interrupts/idt.hpp>
#include <debug/debug.hpp>
#include <interrupts/isr.hpp>

namespace meta
{
    template<uint32_t I>
    class idt_loop {
    private:
        enum { go = (I-1) != 0 };

    public:
        static inline void f(idt_entry* entries)
        {
            entries[I - 1] = idt_entry(isr_unhandled<I - 1>, CODE_SEGMENT,
                              idt_entry_type::INTERRUPT_32,
                              idt_entry_flags::PRESENT);
            idt_loop<go ? (I-1) : 0>::f(entries);
        }
    };
    template<>
    class idt_loop<0> {
    public:
        static inline void f(idt_entry*)
        { }
    };
}

#endif /* LOOP_HPP */