#ifndef CTHUSLY_PROGRAM_H
#define CTHUSLY_PROGRAM_H

#include "common.h"
#include "memory.h"

typedef enum {
  OP_RETURN,
} OpCode;

typedef struct {
  byte* instructions;
  int count;
  int capacity;
} Program;

void init_program(Program* program);
void free_program(Program* program);
void append_instruction(Program* program, byte instruction);

#endif
