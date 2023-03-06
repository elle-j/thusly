#ifndef CTHUSLY_VM_H
#define CTHUSLY_VM_H

#include "program.h"

typedef struct {
  Program* program;
  byte* next_instruction;
} VM;

typedef enum {
  REPORT_NO_ERROR,
  REPORT_COMPILE_ERROR,
  REPORT_RUNTIME_ERROR,
} ErrorReport;

void init_vm(VM* vm);
void free_vm(VM* vm);
ErrorReport interpret(VM* vm, Program* program);

#endif
