#ifndef ISR_HPP
#define ISR_HPP

#include <stdint.h>

struct interrupt_frame
{
    uint32_t eip   ;
    uint32_t cs    ;
    uint32_t eflags;
    uint32_t esp   ;
    uint32_t ss    ;
} __attribute__((packed));

void isr_keyboard( interrupt_frame*);
void isr_unhandled(interrupt_frame*);


#endif /* ISR_HPP */