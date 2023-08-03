#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "gc_object.h"
#include "memory.h"
#include "vm.h"

static void reset_stack(VM* vm) {
  vm->next_stack_top = vm->stack;
}

void vm_init(VM* vm) {
  reset_stack(vm);
  vm->environment.vm = vm;
  vm->environment.gc_objects = NULL;
  vm->program = NULL;
  table_init(&vm->environment.texts);
}

void vm_free(VM* vm) {
  // -- TEMPORARY --
  #ifdef DEBUG_EXECUTION
    printf("FREEING VM..\n");
  #endif
  // ---------------

  vm->program = NULL;
  table_free(&vm->environment.texts);
  free_objects(&vm->environment);
}

static void error(VM* vm, const char* message, ...) {
  // The instructions and source_lines array indexes mirror each other.
  size_t instruction_index = vm->next_instruction - vm->program->instructions - 1;
  int source_line = vm->program->source_lines[instruction_index];

  fprintf(stderr, "\n---------");
  fprintf(stderr, "\n| error |");
  fprintf(stderr, "\n---------");
  fprintf(stderr, "\n\t> Line:\n\t\t%d", source_line);
  fprintf(stderr, "\n\t> What's wrong:\n\t\t");
  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  fputs("\n", stderr);

  reset_stack(vm);
}

static void push(VM* vm, ThuslyValue value) {
  *vm->next_stack_top = value;
  vm->next_stack_top++;

  // TODO: Check size before pushing to prevent stack overflow
}

static ThuslyValue pop(VM* vm) {
  vm->next_stack_top--;
  
  return *vm->next_stack_top;

  // TODO: Check empty
}

static ThuslyValue pop_n(VM* vm, int n) {
  vm->next_stack_top -= n;

  return *vm->next_stack_top;
}

static ThuslyValue peek(VM* vm, int offset) {
  // The current stack top is 1 before next_stack_top. Thus, if the offset passed
  // is 0, this should peek at next_stack_top[-1].
  return vm->next_stack_top[-1 - offset];
}

static bool is_truthy(ThuslyValue value) {
  // At the current stage of implementation, all values, including 0, are considered
  // truthy except for: none, false.
  return !(IS_NONE(value) || (IS_BOOLEAN(value) && !TO_C_BOOL(value)));
}

static void concatenate(VM* vm) {
  TextObject* b = TO_TEXT(pop(vm));
  TextObject* a = TO_TEXT(pop(vm));
  int length = a->length + b->length;
  // Allocate +1 for the terminating null byte.
  char* chars_concatenated = ALLOCATE(char, length + 1);
  memcpy(chars_concatenated, a->chars, a->length);
  memcpy(chars_concatenated + a->length, b->chars, b->length);
  chars_concatenated[length] = '\0';

  TextObject* result = claim_c_string(&vm->environment, chars_concatenated, length);
  push(vm, FROM_C_OBJECT_PTR(result));
}

static ErrorReport decode_and_execute(VM* vm) {
  #define READ_BYTE()     (*vm->next_instruction++)
  // Moves the instruction pointer past the jump operand (2 bytes) and returns it as unsigned.
  #define READ_SHORT()    (vm->next_instruction += 2, (uint16_t)((vm->next_instruction[-2] << 8) | vm->next_instruction[-1]))
  #define READ_CONSTANT() (vm->program->constant_pool.values[READ_BYTE()])

  #define DO_BINARY_OP(from_c_value, operator, operator_string)                             \
    do {                                                                                    \
      if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {                             \
        error(vm, "The operation (%s) can only be performed on numbers.", operator_string); \
        return REPORT_RUNTIME_ERROR;                                                        \
      }                                                                                     \
      double b = TO_C_DOUBLE(pop(vm));                                                      \
      double a = TO_C_DOUBLE(pop(vm));                                                      \
      push(vm, from_c_value(a operator b));                                                 \
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
      case OP_POP:
        pop(vm);
        break;
      case OP_POPN: {
        byte n = READ_BYTE();
        // `N` in `POPN` is treated as "the number to pop minus 1" in order to allow
        // popping the maximum number of variables supported on the stack (UINT8_MAX + 1,
        // i.e. 256). Therefore, it is incremented by 1 here.
        pop_n(vm, n + 1);
        break;
      }
      case OP_GET_VAR: {
        byte slot = READ_BYTE();
        // Since this is a stack-based VM, instructions will rely on values
        // being at the top of stack. Therefore, the value at the given slot
        // is also pushed to the top.
        push(vm, vm->stack[slot]);
        break;
      }
      case OP_SET_VAR: {
        byte slot = READ_BYTE();
        // An assignment expression evaluates to the assigned value.
        // Therefore, the value is not popped from the stack.
        vm->stack[slot] = peek(vm, 0);
        break;
      }
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
      case OP_EQUALS: {
        ThuslyValue b = pop(vm);
        ThuslyValue a = pop(vm);
        push(vm, FROM_C_BOOL(values_are_equal(a, b)));
        break;
      }
      case OP_NOT_EQUALS: {
        ThuslyValue b = pop(vm);
        ThuslyValue a = pop(vm);
        push(vm, FROM_C_BOOL(!values_are_equal(a, b)));
        break;
      }
      case OP_GREATER_THAN:
        DO_BINARY_OP(FROM_C_BOOL, >, ">");
        break;
      case OP_GREATER_THAN_EQUALS:
        DO_BINARY_OP(FROM_C_BOOL, >=, ">=");
        break;
      case OP_LESS_THAN:
        DO_BINARY_OP(FROM_C_BOOL, <, "<");
        break;
      case OP_LESS_THAN_EQUALS:
        DO_BINARY_OP(FROM_C_BOOL, <=, "<=");
        break;
      case OP_ADD: {
        if (IS_TEXT(peek(vm, 0)) && IS_TEXT(peek(vm, 1)))
          concatenate(vm);
        else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
          double b = TO_C_DOUBLE(pop(vm));
          double a = TO_C_DOUBLE(pop(vm));
          push(vm, FROM_C_DOUBLE(a + b));
        }
        else {
          error(vm, "Addition/concatenation (+) can only be performed on either numbers or texts.");
          return REPORT_RUNTIME_ERROR;
        }
        break;
      }
      case OP_SUBTRACT:
        DO_BINARY_OP(FROM_C_DOUBLE, -, "-");
        break;
      case OP_MULTIPLY:
        DO_BINARY_OP(FROM_C_DOUBLE, *, "*");
        break;
      case OP_DIVIDE:
        DO_BINARY_OP(FROM_C_DOUBLE, /, "/");
        break;
        // TODO: Handle division by 0
      case OP_MODULO: {
        if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {
          error(vm, "Modulo (\%) can only be performed on numbers.");
          return REPORT_RUNTIME_ERROR;
        }
        double b = TO_C_DOUBLE(pop(vm));
        double a = TO_C_DOUBLE(pop(vm));
        push(vm, FROM_C_DOUBLE(fmod(a, b)));
        break;
        // TODO: Handle division by 0
      }
      case OP_NEGATE:
        // Peek at the stack rather than pop here in case there is garbage
        // collection before the value is pushed onto the stack again.
        if (!IS_NUMBER(peek(vm, 0))) {
          error(vm, "Negation (-) can only be performed on numbers.");
          return REPORT_RUNTIME_ERROR;
        }
        push(vm, FROM_C_DOUBLE(-TO_C_DOUBLE(pop(vm))));
        break;
      case OP_NOT:
        push(vm, FROM_C_BOOL(!is_truthy(pop(vm))));
        break;
      case OP_OUT:
        print_value(pop(vm));
        printf("\n");
        break;
      case OP_JUMP_FWD: {
        uint16_t offset = READ_SHORT();
        vm->next_instruction += offset;
        break;
      }
      case OP_JUMP_FWD_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (!is_truthy(peek(vm, 0)))
          vm->next_instruction += offset;
        break;
      }
      case OP_RETURN: {
        return REPORT_NO_ERROR;
      }
    }
  }

  #undef READ_BYTE
  #undef READ_SHORT
  #undef READ_CONSTANT
  #undef DO_BINARY_OP
}

ErrorReport interpret(VM* vm, const char* source) {
  Program program;
  program_init(&program);

  bool saw_error = !compile(&vm->environment, source, &program);
  if (saw_error) {
    program_free(&program);
    return REPORT_COMPILE_ERROR;
  }

  vm->program = &program;
  vm->next_instruction = program.instructions;
  ErrorReport report = decode_and_execute(vm);

  // TODO: Since instructions of the program are freed after each
  //       `interpret`, variables used in the REPL will not be usable.
  program_free(&program);

  return report;
}
