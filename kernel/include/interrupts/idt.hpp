#ifndef IDT_HPP
#define IDT_HPP

#include <interrupts/isr.hpp>

#include <stdint.h>

typedef void(*interrupt_handler)(interrupt_frame*);
typedef void(*exception_hanlder)(interrupt_frame*, uint32_t);

enum idt_entry_type
{
    TASK         = 0b00000101,
    INTERRUPT_16 = 0b00000110,
    INTERRUPT_32 = 0b00001110,
    TRAP_16      = 0b00000111,
    TRAP_32      = 0b00001111
};

enum idt_entry_flags
{
    PRESENT = 0b10000000,
    DPL_1   = 0b00100000,
    DPL_2   = 0b01000000,
    DPL_3   = 0b01100000,
    SEGMENT = 0b00010000
};

class idt_entry
{
private:
   uint16_t offset_low ; // offset bits 0..15
   uint16_t selector   ; // a code segment selector in GDT or LDT
   uint8_t  zero       ; // unused, set to 0
   uint8_t  type_attr  ; // type and attributes, see below
   uint16_t offset_high; // offset bits 16..31
public:
    idt_entry();
    idt_entry(interrupt_handler, uint16_t segment,
              idt_entry_type type, uint8_t flags);
    idt_entry(exception_hanlder, uint16_t segment,
        idt_entry_type type, uint8_t flags);
} __attribute__((packed));

struct idt
{
    uint16_t   size;
    idt_entry* base;
} __attribute__((packed));

extern "C" void load_idt(idt);
void setup_idt();

#endif /* IDT_HPP */