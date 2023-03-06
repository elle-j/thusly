#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "program.h"
#include "thusly_value.h"
#include "vm.h"

static void reset_stack(VM* vm) {
  vm->next_stack_top = vm->stack;
}

void init_vm(VM* vm) {
  reset_stack(vm);
}

void free_vm(VM* vm) {
  // TODO

  printf("FREEING VM..\n"); // TEMPORARY
}

void push(VM* vm, ThuslyValue value) {
  *vm->next_stack_top = value;
  vm->next_stack_top++;

  // TODO: Check stack overflow
}

ThuslyValue pop(VM* vm) {
  vm->next_stack_top--;
  
  return *vm->next_stack_top;

  // TODO: Check empty
}

typedef double (*BinaryOp)(double a, double b);

static inline double op_add(double a, double b) {
  return a + b;
}

static inline double op_divide(double a, double b) {
  return a / b;
}

static inline double op_multiply(double a, double b) {
  return a * b;
}

static inline double op_subtract(double a, double b) {
  return a - b;
}

static inline void binary_op(VM* vm, BinaryOp op) {
  {
    double b = pop(vm);
    double a = pop(vm);
    push(vm, op(a, b));
  }
}

static ErrorReport decode_and_execute(VM* vm) {
  #define READ_BYTE() (*vm->next_instruction++)
  #define READ_CONSTANT() (vm->program->constant_pool.values[READ_BYTE()])

  while (true) {
    #ifdef DEBUG_EXECUTION
    {
      disassemble_stack(vm);
      int offset = (int)(vm->next_instruction - vm->program->instructions);
      disassemble_instruction(vm->program, offset);
    }
    #endif

    byte instruction = READ_BYTE();
    switch (instruction) {
      case OP_CONSTANT: {
        ThuslyValue constant = READ_CONSTANT();
        push(vm, constant);
        break;
      }
      case OP_ADD:
        binary_op(vm, op_add);
        break;
      case OP_DIVIDE:
        binary_op(vm, op_divide);
        break;
      case OP_MULTIPLY:
        binary_op(vm, op_multiply);
        break;
      case OP_SUBTRACT:
        binary_op(vm, op_subtract);
        break;
      case OP_NEGATE:
        // Can temporarily use `-` directly since only treating ThuslyValues as doubles for now.
        push(vm, -pop(vm));
        break;
      case OP_RETURN: {
        printf("> Result: ");   // Temporary
        print_value(pop(vm));
        printf("\n");
        return REPORT_NO_ERROR;
      }
    }
  }

  #undef READ_BYTE
  #undef READ_CONSTANT
}

ErrorReport interpret(VM* vm, Program* program) {
  vm->program = program;
  vm->next_instruction = vm->program->instructions;

  return decode_and_execute(vm);
}
