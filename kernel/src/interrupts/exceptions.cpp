#include <interrupts/exceptions.hpp>
#include <debug/debug.hpp>

void e_page_fault(interrupt_frame*, uint32_t error_code)
{
    dbg << "Page Fault: " << error_code << '\n';
    DEBUG_BREAK;
}

void e_general_protection(interrupt_frame*, uint32_t error_code)
{
    dbg << "General Protection: " << error_code << '\n';
    DEBUG_BREAK;
}