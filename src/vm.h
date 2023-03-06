#ifndef CTHUSLY_VM_H
#define CTHUSLY_VM_H

#include "program.h"
#include "thusly_value.h"

#define STACK_MAX 256

typedef struct {
  Program* program;
  byte* next_instruction;
  ThuslyValue stack[STACK_MAX];
  // When pointing to zeroth element, the stack is empty.
  ThuslyValue* next_stack_top;
} VM;

typedef enum {
  REPORT_NO_ERROR,
  REPORT_COMPILE_ERROR,
  REPORT_RUNTIME_ERROR,
} ErrorReport;

void init_vm(VM* vm);
void free_vm(VM* vm);
void push(VM* vm, ThuslyValue value);
ThuslyValue pop(VM* vm);
ErrorReport interpret(VM* vm, Program* program);

#endif
