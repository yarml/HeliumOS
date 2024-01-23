#ifndef HELIUM_INT_HANDLERS_H
#define HELIUM_INT_HANDLERS_H

#include <interrupts.h>
#include <stdint.h>

void inter_unmapped(int_frame *frame);
void exception_div(int_frame *frame);
void exception_page_fault(int_frame *frame, uint64_t err_code);
void exception_stackseg_fault(int_frame *frame, uint64_t ec);
void exception_general_prot(int_frame *frame, uint64_t ec);
void exception_double_fault(int_frame *frame, uint64_t ec);

void apic_err(int_frame *frame);
void timer_tick(int_frame *frame);
void spurious_int(int_frame *frame);

#endif