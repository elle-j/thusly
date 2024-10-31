#ifndef CTHUSLY_VM_H
#define CTHUSLY_VM_H

#include "program.h"
#include "table.h"
#include "thusly_value.h"

#define STACK_MAX (UINT8_MAX + 1)

struct VM;

/// Heap data used by the VM.
typedef struct {
  struct VM* vm;
  /// Heap-allocated objects (points to the head).
  GCObject* gc_objects;
  /// The text (string) intern pool. All texts created are interned
  /// and added to this pool. (Only the keys in this table are used,
  /// so the values will all be `none` ThuslyValues.)
  Table texts;
} Environment;

/// The virtual machine - Interprets and executes the instructions
/// in the compiled program in a sequential order.
typedef struct VM {
  Environment environment;
  Program* program;
  /// The next instruction to be executed.
  byte* next_instruction;
  /// The last-in-first-out (LIFO) operand stack used for the operands of the
  /// operators used, as well as for the result of an evaluated expression.
  ThuslyValue stack[STACK_MAX];
  /// The next slot for the top of the stack.
  /// (When pointing to the zeroth element, the stack is empty.)
  ThuslyValue* next_stack_top;
} VM;

/// The error report from interpreting the source file, used
/// for determining whether to continue or exit the process.
typedef enum {
  REPORT_NO_ERROR,
  REPORT_COMPILE_ERROR,
  REPORT_RUNTIME_ERROR,
} ErrorReport;

void vm_init(VM* vm);
void vm_free(VM* vm);
ErrorReport interpret(VM* vm, const char* source);

#endif
