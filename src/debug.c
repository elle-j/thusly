#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "program.h"
#include "thusly_value.h"

#define COLUMN_LENGTH 12

void disassembler_print_headings(const char* title) {
  printf("================ %s ================\n\n", title);
  printf("Source      Bytecode    Bytecode\n");
  printf("Line        Offset      Instruction");
  printf("\n\n");
}

static void indent(int size) {
  printf("%*c", size, ' ');
}

void disassembler_indent_to_last_column() {
  indent(COLUMN_LENGTH * 2);
}

static void print_source_line_number(Program* program, int offset) {
  bool is_same_line_as_previous = offset > 0 && program->source_lines[offset] == program->source_lines[offset - 1];
  if (is_same_line_as_previous)
    indent(COLUMN_LENGTH);
  else {
    printf("%-4d", program->source_lines[offset]);
    indent(COLUMN_LENGTH - 4);
  }
}

static void print_bytecode_offset(int offset) {
  printf("%-4d", offset);
  indent(COLUMN_LENGTH - 4);
}

static int print_opcode(const char* op_name, int offset) {
  printf("%s\n", op_name);

  return offset + 1;
}

static int print_pop_n(const char* op_name, Program* program, int offset) {
  byte number = program->instructions[offset + 1];
  // See `compiler.discard_scope()` for comments regarding `number + 1`.
  printf("%s %d        (pops %d + 1 values)\n", op_name, number, number);

  return offset + 2;
}

static int print_constant(const char* op_name, Program* program, int offset) {
  byte constant_index = program->instructions[offset + 1]; 
  // printf("%-16s %d (points to: ", op_name, constant_index);
  printf("%s %d    (points to: ", op_name, constant_index);
  print_value(program->constant_pool.values[constant_index]);
  printf(")\n");

  return offset + 2;
}

static int print_variable(const char* op_name, Program* program, int offset) {
  // This currently only prints the stack slot of the variable as the
  // names of the variables are not stored in the program. (Only the
  // values exist on the stack.)
  byte variable_slot = program->instructions[offset + 1];
  printf("%s %d\n", op_name, variable_slot);

  return offset + 2;
}

static int print_jump(const char* op_name, Program* program, int sign, int offset) {
  uint16_t jump_offset = (uint16_t)((program->instructions[offset + 1] << 8) | program->instructions[offset + 2]);
  int target_offset = offset + 3 + sign * jump_offset;
  printf("%s %d    (jumps from %d to %d)\n", op_name, jump_offset, offset, target_offset);

  return offset + 3;
}

/// Disassemble the instruction and return the offset to the next instruction.
int disassemble_instruction(Program* program, int offset) {
  print_source_line_number(program, offset);
  print_bytecode_offset(offset);

  byte instruction = program->instructions[offset];
  switch (instruction) {
    case OP_POP:
      return print_opcode("OP_POP", offset);
    case OP_POPN:
      return print_pop_n("OP_POPN", program, offset);
    case OP_GET_VAR:
      return print_variable("OP_GET_VAR", program, offset);
    case OP_SET_VAR:
      return print_variable("OP_SET_VAR", program, offset);
    case OP_CONSTANT:
      return print_constant("OP_CONSTANT", program, offset);
    case OP_CONSTANT_FALSE:
      return print_opcode("OP_CONSTANT_FALSE", offset);
    case OP_CONSTANT_NONE:
      return print_opcode("OP_CONSTANT_NONE", offset);
    case OP_CONSTANT_TRUE:
      return print_opcode("OP_CONSTANT_TRUE", offset);
    case OP_EQUALS:
      return print_opcode("OP_EQUALS", offset);
    case OP_NOT_EQUALS:
      return print_opcode("OP_NOT_EQUALS", offset);
    case OP_GREATER_THAN:
      return print_opcode("OP_GREATER_THAN", offset);
    case OP_GREATER_THAN_EQUALS:
      return print_opcode("OP_GREATER_THAN_EQUALS", offset);
    case OP_LESS_THAN:
      return print_opcode("OP_LESS_THAN", offset);
    case OP_LESS_THAN_EQUALS:
      return print_opcode("OP_LESS_THAN_EQUALS", offset);
    case OP_ADD:
      return print_opcode("OP_ADD", offset);
    case OP_SUBTRACT:
      return print_opcode("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return print_opcode("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return print_opcode("OP_DIVIDE", offset);
    case OP_MODULO:
      return print_opcode("OP_MODULO", offset);
    case OP_NEGATE:
      return print_opcode("OP_NEGATE", offset);
    case OP_NOT:
      return print_opcode("OP_NOT", offset);
    case OP_OUT:
      return print_opcode("OP_OUT", offset);
    case OP_JUMP_FWD:
      return print_jump("OP_JUMP_FWD", program, 1, offset);
    case OP_JUMP_FWD_IF_FALSE:
      return print_jump("OP_JUMP_FWD_IF_FALSE", program, 1, offset);
    case OP_JUMP_FWD_IF_TRUE:
      return print_jump("OP_JUMP_FWD_IF_TRUE", program, 1, offset);
    case OP_JUMP_BWD:
      return print_jump("OP_JUMP_BWD", program, -1, offset);
    case OP_RETURN:
      return print_opcode("OP_RETURN", offset);
    default:
      printf("Unsupported opcode %d\n", instruction);
      return offset + 1;
  }
}

void disassemble_program(Program* program) {
  disassembler_print_headings("Program");

  int offset = 0;
  while (offset < program->count)
    offset = disassemble_instruction(program, offset);

  printf("\n");
}

void disassemble_stack(VM* vm) {
  disassembler_indent_to_last_column();
  printf("stack: [");
  for (ThuslyValue* stack_elem_ptr = vm->stack; stack_elem_ptr < vm->next_stack_top; stack_elem_ptr++) {
    print_value(*stack_elem_ptr);
    bool is_last = stack_elem_ptr + 1 == vm->next_stack_top;
    if (!is_last)
      printf(", ");
  }
  printf("]\n\n");
}
