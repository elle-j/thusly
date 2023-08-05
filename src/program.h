#ifndef CTHUSLY_PROGRAM_H
#define CTHUSLY_PROGRAM_H

#include "common.h"
#include "thusly_value.h"

typedef enum {
  OP_ADD,
  OP_CONSTANT,
  // Dedicated operations for common constants.
  OP_CONSTANT_FALSE,
  OP_CONSTANT_NONE,
  OP_CONSTANT_TRUE,
  // ----
  OP_DIVIDE,
  OP_EQUALS,
  OP_GET_VAR,
  OP_GREATER_THAN,
  OP_GREATER_THAN_EQUALS,
  OP_JUMP_BWD,
  OP_JUMP_FWD,
  OP_JUMP_FWD_IF_FALSE,
  OP_JUMP_FWD_IF_TRUE,
  OP_LESS_THAN,
  OP_LESS_THAN_EQUALS,
  OP_MODULO,
  OP_MULTIPLY,
  OP_NEGATE,
  OP_NOT,
  OP_NOT_EQUALS,
  OP_OUT,
  OP_POP,
  OP_POPN,
  OP_RETURN,
  OP_SET_VAR,
  OP_SUBTRACT,
} Opcode;

typedef struct {
  ThuslyValue* values;
  int count;
  int capacity;
} ConstantPool;

typedef struct {
  ConstantPool constant_pool;
  int* source_lines; // Mirrors the `instructions`
  byte* instructions;
  int count;
  int capacity;
} Program;

void program_init(Program* program);
void program_free(Program* program);
void program_write(Program* program, byte instruction, int source_line);
void program_overwrite(Program* program, int offset, byte updated_instruction);
int program_add_constant(Program* program, ThuslyValue value);

#endif
