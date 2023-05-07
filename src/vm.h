#ifndef CTHUSLY_VM_H
#define CTHUSLY_VM_H

#include "program.h"
#include "table.h"
#include "thusly_value.h"

#define STACK_MAX 256

struct VM;

/// Heap data used by the VM.
typedef struct {
  struct VM* vm;
  // Heap-allocated objects (points to the head).
  GCObject* gc_objects;
  // All texts (strings) created are interned. (Only the keys in this
  // table are used, so the values will all be `none` ThuslyValues.)
  Table texts;
} Environment;

/// The virtual machine interpreting and executing the instructions in
/// the compiled program in a sequential order.
typedef struct VM {
  Environment environment;
  Program* program;
  byte* next_instruction;
  ThuslyValue stack[STACK_MAX];
  // When pointing to the zeroth element, the stack is empty.
  ThuslyValue* next_stack_top;
} VM;

typedef enum {
  REPORT_NO_ERROR,
  REPORT_COMPILE_ERROR,
  REPORT_RUNTIME_ERROR,
} ErrorReport;

void vm_init(VM* vm);
void vm_free(VM* vm);
ErrorReport interpret(VM* vm, const char* source);

#endif
