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

static ErrorReport decode_and_execute(VM* vm) {
  #define READ_BYTE() (*vm->next_instruction++)

  #define READ_CONSTANT() (vm->program->constant_pool.values[READ_BYTE()])

  #define DO_BINARY_OP(from_c_value, operator)                        \
    do {                                                              \
      if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {       \
        error(vm, "The operation can only be performed on numbers."); \
        return REPORT_RUNTIME_ERROR;                                  \
      }                                                               \
      double b = TO_C_DOUBLE(pop(vm));                                \
      double a = TO_C_DOUBLE(pop(vm));                                \
      push(vm, from_c_value(a operator b));                           \
    } while (false)

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
      case OP_CONSTANT_FALSE:
        push(vm, FROM_C_BOOL(false));
        break;
      case OP_CONSTANT_NONE:
        push(vm, FROM_C_NULL);
        break;
      case OP_CONSTANT_TRUE:
        push(vm, FROM_C_BOOL(true));
        break;
      case OP_ADD:
        DO_BINARY_OP(FROM_C_DOUBLE, +);
        break;
      case OP_DIVIDE:
        DO_BINARY_OP(FROM_C_DOUBLE, /);
        break;
      case OP_MULTIPLY:
        DO_BINARY_OP(FROM_C_DOUBLE, *);
        break;
      case OP_SUBTRACT:
        DO_BINARY_OP(FROM_C_DOUBLE, -);
        break;
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
  #undef DO_BINARY_OP
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
