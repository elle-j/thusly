#ifndef CTHUSLY_PROGRAM_H
#define CTHUSLY_PROGRAM_H

#include "common.h"
#include "memory.h"
#include "thusly_value.h"

typedef enum {
  OP_ADD,
  OP_CONSTANT,
  OP_CONSTANT_FALSE,
  OP_CONSTANT_NONE,
  OP_CONSTANT_TRUE,
  OP_DIVIDE,
  OP_MULTIPLY,
  OP_NEGATE,
  OP_RETURN,
  OP_SUBTRACT,
} Opcode;

typedef struct {
  ConstantPool constant_pool;
  int* source_lines; // Mirrors the `instructions`
  byte* instructions;
  int count;
  int capacity;
} Program;

void init_program(Program* program);
void free_program(Program* program);
void append_instruction(Program* program, byte instruction, int source_line);
int add_constant(Program* program, ThuslyValue value);

#endif
