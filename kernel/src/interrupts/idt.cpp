#include <stdint.h>
#include <debug/debug.hpp>
#include <dev/pic.hpp>
#include <interrupts/idt.hpp>
#include <interrupts/isr.hpp>
#include <meta/loop.hpp>

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

static idt_entry idt_entries[256];
static idt       idtr            ;

void init_idt_entries(idt_entry* e);

void setup_idt()
{
    dbg << "setting up idt\n";
    pic::remap(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET);
    for(int i = 0; i < 16; i++)
        pic::set_mask(i);
    
    init_idt_entries(idt_entries);
    
    // Keyboard
    idt_entries[PIC_MASTER_OFFSET + irq::KEYBOARD] = idt_entry(isr_keyboard, CODE_SEGMENT,
                                                           idt_entry_type::INTERRUPT_32,
                                                           idt_entry_flags::PRESENT);
    
    // PIT
    idt_entries[PIC_MASTER_OFFSET + irq::PIT] = idt_entry(isr_pit, CODE_SEGMENT,
                                                           idt_entry_type::INTERRUPT_32,
                                                           idt_entry_flags::PRESENT);
    
    idtr = {.size = sizeof(idt_entries), .base = idt_entries};
    load_idt(idtr);
    pic::clear_mask(irq::PIT);
    pic::clear_mask(irq::KEYBOARD);
    pic::clear_mask(irq::PS_MOUSE);

    dbg << "setup idt\n";
}
