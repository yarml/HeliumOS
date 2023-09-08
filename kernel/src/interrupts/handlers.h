#ifndef HELIUM_INT_HANDLERS_H
#define HELIUM_INT_HANDLERS_H

#include <interrupts.h>
#include <stdint.h>

void exception_div(int_frame *frame);
void exception_page_fault(int_frame *frame, int_errcode_pf err_code);
void exception_stackseg_fault(int_frame *frame, uint64_t ec);
void exception_general_prot(int_frame *frame, uint64_t ec);
void exception_double_fault(int_frame *frame, uint64_t ec);

#endif