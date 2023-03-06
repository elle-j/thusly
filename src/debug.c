#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "program.h"

static int print_op_code(const char* name, int offset) {
  printf("op[%s]\n", name);

  return offset + 1;
}

void disassemble_program(Program* program, const char* name) {
  printf("===== %s =====\n", name);

  int offset = 0;
  while (offset < program->count)
    offset = disassemble_instruction(program, offset);
}

int disassemble_instruction(Program* program, int offset) {
  printf("offset[%04d] ", offset);

  byte instruction = program->instructions[offset];
  switch (instruction) {
    case OP_RETURN:
      return print_op_code("OP_RETURN", offset);
    default:
      printf("Unsupported opcode %d\n", instruction);
      return offset + 1;
  }
}
