#include <stdint.h>
#include <debug/debug.hpp>
#include <dev/pic.hpp>
#include <interrupts/idt.hpp>
#include <interrupts/isr.hpp>

idt_entry::idt_entry() {}

idt_entry::idt_entry(void (*handler)(interrupt_frame*), uint16_t segment,
              idt_entry_type type, uint8_t flags)
{
    offset_low  = ((uint32_t) handler      ) & 0x0000FFFF;
    offset_high = ((uint32_t) handler >> 16) & 0x0000FFFF;
    selector    = segment                                ;
    type_attr   = flags | type                           ;
    zero        = 0                                      ;
}

static idt_entry entries[256];
static idt       idtr        ;

void setup_idt()
{
    dbg << "setting up idt\n";
    pic::remap(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET);
    for(int i = 0; i < 16; i++)
        pic::set_mask(i);
    for(uint32_t i = 0; i < sizeof(entries) / sizeof(idt_entry); i++)
        entries[i] = idt_entry(isr_unhandled, 0x0008,
                              idt_entry_type::INTERRUPT_32,
                              idt_entry_flags::PRESENT);
    

    // Keyboard
    pic::clear_mask(irq::KEYBOARD);
    entries[PIC_MASTER_OFFSET + irq::KEYBOARD] = idt_entry(isr_keyboard, 0x0008,
                                                           idt_entry_type::INTERRUPT_32,
                                                           idt_entry_flags::PRESENT);

    idtr = {.size = sizeof(entries), .base = entries};
    load_idt(idtr);
    dbg << "setup idt\n";
}
