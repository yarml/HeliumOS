#include <apic.h>
#include <interrupts.h>
#include <kterm.h>
#include <mutex.h>
#include <proc.h>
#include <stdio.h>
#include <sys.h>

#include <asm/ctlr.h>
#include <asm/io.h>
#include <dev/ps2.h>

static void exception_common_prologue(int_frame *frame, char *name) {
  printd("[Proc %&] [EXCEPTION:%s]\n", name);
  printd(
      "IP: %016lx\n"
      "CS: %016lx\n"
      "SS: %016lx\n"
      "RF: %016lx\n"
      "SP: %016lx\n",
      frame->ip,
      frame->cs,
      frame->ss,
      frame->flags,
      frame->sp
  );
}

interrupt_handler void inter_unmapped(int_frame *frame) {
  printd("[Proc %&] Received unmapped interrupt\n");
  stop();
}

interrupt_handler void exception_div(int_frame *frame) {
  exception_common_prologue(frame, "DIV ERROR");

  stop();
}

interrupt_handler void exception_page_fault(int_frame *frame, uint64_t ec) {
  int_errcode_pf *err_code = (void *)&ec;

  uint64_t adr = as_rcr2();

  exception_common_prologue(frame, "PAGE FAULT");

  char *operation = err_code->write ? "write" : "read";
  char *priv      = err_code->user ? "user" : "kernel";

  printd(
      "Memory violation while trying to %s.\n"
      "Running code is %s.\n"
      "At memory address %p\n",
      operation,
      priv,
      adr
  );

  if (!err_code->present) {
    printd("Caused by structure without present flag.\n");
  } else if (err_code->rsvd) {
    printd("Caused by reserved bit set to 1.\n");
  } else {
    printd("Caused by page level protection.\n");
  }

  if (err_code->pk_violation) {
    printd("Caused by protection-key violation.\n");
  }

  if (err_code->shadow_stack) {
    printd("Caused by shadow stack.\n");
  }

  if (err_code->ins_fetch) {
    printd("While trying to fetch instruction.\n");
  }

  if (err_code->hlat) {
    printd("HLAT.\n");
  }

  if (err_code->sgx) {
    printd("SGX.\n");
  }

  // For the far far far far far far far future, implement swapping here
  stop();
}

interrupt_handler void exception_stackseg_fault(int_frame *frame, uint64_t ec) {
  exception_common_prologue(frame, "STACK SEGFAULT");

  printd("Error code: %08lx\n", ec);

  stop();
}

interrupt_handler void exception_general_prot(int_frame *frame, uint64_t ec) {
  exception_common_prologue(frame, "GENERAL PROTECTION");

  printd("Error code: %08lx\n", ec);

  stop();
}

interrupt_handler void exception_double_fault(int_frame *frame, uint64_t ec) {
  exception_common_prologue(frame, "DOUBLE FAULT");

  printd("Error code: %08lx\n", ec);

  stop();
}

static mutex timer_lock;

interrupt_handler void apic_err(int_frame *frame) {
  printf("[Proc %&] APIC Error\n");
  stop();
}

interrupt_handler void timer_tick(int_frame *frame) {
  // No printing, too much noise
  kterm_flush();
  APIC_VBASE->eoireg[0] = 0;
}

interrupt_handler void spurious_int(int_frame *frame) {
  printf("Spurious\n");
  APIC_VBASE->eoireg[0] = 0;
}
