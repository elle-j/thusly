#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "program.h"
#include "thusly_value.h"
#include "vm.h"

static void reset_stack(VM* vm) {
  vm->next_stack_top = vm->stack;
}

void init_vm(VM* vm) {
  reset_stack(vm);

  // TODO
}

void free_vm(VM* vm) {
  // TODO

  printf("FREEING VM..\n"); // TEMPORARY
}

static void error(VM* vm, const char* message, ...) {
  // The instructions and source_lines array indexes mirror each other.
  size_t instruction_index = vm->next_instruction - vm->program->instructions - 1;
  int source_line = vm->program->source_lines[instruction_index];

  fprintf(stderr, "ERROR on line %d:\n", source_line);
  fprintf(stderr, "\t>> Help: ");
  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  fputs("\n", stderr);

  reset_stack(vm);
}

void push(VM* vm, ThuslyValue value) {
  *vm->next_stack_top = value;
  vm->next_stack_top++;

  // TODO: Check size before pushing to prevent stack overflow
}

ThuslyValue pop(VM* vm) {
  vm->next_stack_top--;
  
  return *vm->next_stack_top;

  // TODO: Check empty
}

ThuslyValue peek(VM* vm, int offset) {
  // The current stack top is 1 before next_stack_top. Thus, if the offset passed
  // is 0, this should peek at next_stack_top[-1].
  return vm->next_stack_top[-1 - offset];
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

static inline ErrorReport binary_arithmetic(VM* vm, BinaryOp op) {
  {
    if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {
      error(vm, "The operation can only be performed on numbers.");
      return REPORT_RUNTIME_ERROR;
    }
    double b = TO_C_DOUBLE(pop(vm));
    double a = TO_C_DOUBLE(pop(vm));
    push(vm, FROM_C_DOUBLE(op(a, b)));

    return REPORT_NO_ERROR;
  }
}

static ErrorReport decode_and_execute(VM* vm) {
  #define READ_BYTE() (*vm->next_instruction++)
  #define READ_CONSTANT() (vm->program->constant_pool.values[READ_BYTE()])

  #ifdef DEBUG_EXECUTION
  printf("========== Execution ==========\n");
  #endif

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
      case OP_ADD: {
        ErrorReport report = binary_arithmetic(vm, op_add);
        if (report == REPORT_RUNTIME_ERROR)
          return report;
        break;
      }
      case OP_DIVIDE: {
        ErrorReport report = binary_arithmetic(vm, op_divide);
        if (report == REPORT_RUNTIME_ERROR)
          return report;
        break;
      }
      case OP_MULTIPLY: {
        ErrorReport report = binary_arithmetic(vm, op_multiply);
        if (report == REPORT_RUNTIME_ERROR)
          return report;
        break;
      }
      case OP_SUBTRACT: {
        ErrorReport report = binary_arithmetic(vm, op_subtract);
        if (report == REPORT_RUNTIME_ERROR)
          return report;
        break;
      }
      case OP_NEGATE:
        // Peek at the stack rather than pop here in case there is garbage
        // collection before the value is pushed onto the stack again.
        if (!IS_NUMBER(peek(vm, 0))) {
          error(vm, "Negating a value can only be performed on numbers.");
          return REPORT_RUNTIME_ERROR;
        }
        push(vm, FROM_C_DOUBLE(-TO_C_DOUBLE(pop(vm))));
        break;
      case OP_RETURN: {
        printf("> Result: ");   // Temporary
        print_value(pop(vm));
        printf("\n\n");
        return REPORT_NO_ERROR;
      }
    }
  }

  #undef READ_BYTE
  #undef READ_CONSTANT
}

ErrorReport interpret(VM* vm, const char* source) {
  Program program;
  init_program(&program);

  bool has_error = !compile(source, &program);
  if (has_error) {
    free_program(&program);
    return REPORT_COMPILE_ERROR;
  }

  vm->program = &program;
  vm->next_instruction = program.instructions;
  ErrorReport report = decode_and_execute(vm);

  free_program(&program);

  return report;
}
