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
