#ifndef INTERRUPT_FRAME_HPP
#define INTERRUPT_FRAME_HPP

#include <stdint.h>

struct interrupt_frame
{
    uint32_t eip   ;
    uint32_t cs    ;
    uint32_t eflags;
    uint32_t esp   ;
    uint32_t ss    ;
} __attribute__((packed));

#endif /* INTERRUPT_FRAME_HPP */