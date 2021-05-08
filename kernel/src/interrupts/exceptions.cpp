#include <interrupts/exceptions.hpp>
#include <debug/debug.hpp>

void e_page_fault(interrupt_frame*, uint32_t error_code)
{
    dbg << "Page fault: " << error_code << '\n';
    DEBUG_BREAK;
}