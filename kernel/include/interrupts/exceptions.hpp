#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdint.h>
#include <interrupts/interrupt_frame.hpp>

void e_page_fault(interrupt_frame*, uint32_t);

#endif /* EXCEPTIONS_HPP */