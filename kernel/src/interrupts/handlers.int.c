#include <interrupts.h>
#include <stdio.h>
#include <sys.h>

interrupt_handler void div_err(int_frame* frame)
{
  printf("Am I alive????");

  printf(
    "This is what I got:\n"
    "IP: %#016lx\n"
    "CS: %#016lx\n"
    "RF: %#016lx\n"
    "SP: %#016lx\n"
    "SS: %#016lx\n",
    frame->ip,
    frame->cs,
    frame->flags,
    frame->sp,
    frame->ss
  );

  stop();
}