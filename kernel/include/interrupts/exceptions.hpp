#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdint.h>
#include <interrupts/interrupt_frame.hpp>

void e_page_fault(interrupt_frame*, uint32_t);
void e_general_protection(interrupt_frame*, uint32_t error_code);

#endif /* EXCEPTIONS_HPP */