#include <stdio.h>

#include "common.h"
#include "program.h"
#include "thusly_value.h"
#include "vm.h"

void init_vm(VM* vm) {
  // TODO
}

void free_vm(VM* vm) {
  // TODO
}

static ErrorReport decode_and_execute(VM* vm) {
  #define READ_BYTE() (*vm->next_instruction++)
  #define READ_CONSTANT() (vm->program->constant_pool.values[READ_BYTE()])

  while (true) {
    byte instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        ThuslyValue constant = READ_CONSTANT();
        print_value(constant); // Temporary
        printf("\n");
        break;
      }
      case OP_RETURN: {
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
