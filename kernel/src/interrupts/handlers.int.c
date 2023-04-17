#include <interrupts.h>
#include <stdio.h>
#include <term.h>
#include <sys.h>

#include <asm/ctlr.h>

static void exception_common_prologue(int_frame *frame, char *name)
{
  term_setfg(255, 0, 0);
  fprintf(stderr, "[EXCEPTION:%s]\n", name);
  fprintf(
    stderr,
    "IP: %016lx\n"
    "CS: %016lx\n"
    "RF: %016lx\n"
    "SP: %016lx\n"
    "SS: %016lx\n",
    frame->ip,
    frame->cs,
    frame->flags,
    frame->sp,
    frame->ss
  );
}

interrupt_handler void exception_div(int_frame *frame)
{
  exception_common_prologue(frame, "DIV ERROR");

  stop();
}

interrupt_handler void exception_page_fault(int_frame *frame, uint64_t ec)
{
  int_errcode_pf *err_code = (void *) &ec;

  uint64_t adr = as_rcr2();

  exception_common_prologue(frame, "PAGE FAULT");

  char *operation = err_code->write ? "write" : "read";
  char *priv = err_code->user ? "user" : "kernel";

  fprintf(
    stderr,
    "Memory violation while trying to %s.\n"
    "Running code is %s.\n"
    "At memory address %p\n",
    operation, priv, adr
  );

  if(!err_code->present)
    fprintf(stderr, "Caused by structure without present flag.\n");
  else if(err_code->rsvd)
    fprintf(stderr, "Caused by reserved bit set to 1.\n");
  else
    fprintf(stderr, "Caused by page level protection.\n");

  if(err_code->pk_violation)
    fprintf(stderr, "Caused by protection-key violation.\n");

  if(err_code->shadow_stack)
    fprintf(stderr, "Caused by shadow stack.\n");

  if(err_code->ins_fetch)
    fprintf(stderr, "While trying to fetch instruction.\n");

  if(err_code->hlat)
    fprintf(stderr, "HLAT.\n");

  if(err_code->sgx)
    fprintf(stderr, "SGX.\n");

  // For the far far far far far far far future, implement swapping here
  stop();
}
