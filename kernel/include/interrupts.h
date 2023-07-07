#ifndef HELIUM_INTERRUPTS_H
#define HELIUM_INTERRUPTS_H

#include <attributes.h>
#include <stdint.h>

typedef struct INT_FRAME int_frame;

typedef struct INT_ERRCODE int_errcode;
typedef struct INT_ERRCODE_PF int_errcode_pf;

typedef struct IDT idt;
typedef struct IDT_ENTRY idt_entry;

struct INT_FRAME {
  uint64_t ip;
  uint64_t cs;
  uint64_t flags;
  uint64_t sp;
  uint64_t ss;
} pack;

struct INT_ERRCODE {
  uint64_t external : 1;
  uint64_t desc_type : 1;
  uint64_t ti : 1;
  uint64_t seg_sel_idx : 13;
  uint64_t res0 : 48;
} pack;

struct INT_ERRCODE_PF {
  uint64_t present : 1;
  uint64_t write : 1;
  uint64_t user : 1;
  uint64_t rsvd : 1;  // If I get this I am dumb (I am dumb either way)
  uint64_t ins_fetch : 1;
  uint64_t pk_violation : 1;
  uint64_t shadow_stack : 1;
  uint64_t hlat : 1;  // No clue what this is
  uint64_t res0 : 7;
  uint64_t sgx : 1;  // No clue what this is either, new stuff to learn
  uint64_t res1 : 48;
} pack;

struct IDT {
  uint16_t limit;
  idt_entry *offset;
} pack;

struct IDT_ENTRY {
  uint64_t offset0 : 16;
  uint64_t seg_sel : 16;
  uint64_t ist : 3;
  uint64_t res0 : 5;
  uint64_t type : 4;
  uint64_t res1 : 1;
  uint64_t dpl : 2;
  uint64_t present : 1;
  uint64_t offset1 : 48;
  uint64_t res2 : 32;
} pack;

// 64bit IDT types
#define IDT_TYPE_INT (0xE)
#define IDT_TYPE_TRAP (0xF)

void int_init();

#define int_disable() asm("cli")
#define int_enable() asm("sti")

#endif