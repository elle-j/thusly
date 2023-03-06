#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "program.h"
#include "thusly_value.h"

static int print_opcode(const char* op_name, int offset) {
  printf("op[%s]\n", op_name);

  return offset + 1;
}

static int print_constant(const char* op_name, Program* program, int offset) {
  byte constant_index = program->instructions[offset + 1]; 
  // printf("op[%-16s] index[%4d] value[", op_name, constant_index);
  printf("op[%s] index[%d] value[", op_name, constant_index);
  print_value(program->constant_pool.values[constant_index]);
  printf("]\n");

  return offset + 2;
}

void disassemble_stack(VM* vm) {
  printf("                        stack[");
  for (ThuslyValue* stack_elem_ptr = vm->stack; stack_elem_ptr < vm->next_stack_top; stack_elem_ptr++) {
    print_value(*stack_elem_ptr);
    bool is_last = stack_elem_ptr + 1 == vm->next_stack_top;
    if (!is_last)
      printf(", ");
  }
  printf("]\n");
}

void disassemble_program(Program* program, const char* name) {
  printf("========== %s ==========\n", name);

  int offset = 0;
  while (offset < program->count)
    offset = disassemble_instruction(program, offset);

  printf("\n");
}

int disassemble_instruction(Program* program, int offset) {
  printf("offset[%04d] ", offset);

  bool same_line_as_previous = offset > 0 && program->source_lines[offset] == program->source_lines[offset - 1];
  if (same_line_as_previous)
    printf("           ");
  else
    printf("line[%4d] ", program->source_lines[offset]);

  byte instruction = program->instructions[offset];
  switch (instruction) {
    case OP_CONSTANT:
      return print_constant("OP_CONSTANT", program, offset);
    case OP_ADD:
      return print_opcode("OP_ADD", offset);
    case OP_DIVIDE:
      return print_opcode("OP_DIVIDE", offset);
    case OP_MULTIPLY:
      return print_opcode("OP_MULTIPLY", offset);
    case OP_SUBTRACT:
      return print_opcode("OP_SUBTRACT", offset);
    case OP_NEGATE:
      return print_opcode("OP_NEGATE", offset);
    case OP_RETURN:
      return print_opcode("OP_RETURN", offset);
    default:
      printf("Unsupported opcode %d\n", instruction);
      return offset + 1;
  }
}
