#include <stdlib.h>
#include <stdio.h>

#include "program.h"
#include "thusly_value.h"

void init_program(Program* program) {
  program->source_lines = NULL;
  program->instructions = NULL;
  program->count = 0;
  program->capacity = 0;
  init_constant_pool(&program->constant_pool);
}

void free_program(Program* program) {
  printf("FREEING PROGRAM..\n"); // TEMPORARY

  FREE_ARRAY(int, program->source_lines, program->capacity);
  FREE_ARRAY(byte, program->instructions, program->capacity);
  free_constant_pool(&program->constant_pool);
  init_program(program);
}

void append_instruction(Program* program, byte instruction, int source_line) {
  bool should_grow = program->capacity < program->count + 1;
  if (should_grow) {
    int old_capacity = program->capacity;
    program->capacity = GROW_CAPACITY(old_capacity);
    program->instructions = GROW_ARRAY(byte, program->instructions, old_capacity, program->capacity);
    program->source_lines = GROW_ARRAY(int, program->source_lines, old_capacity, program->capacity);
  }

  program->instructions[program->count] = instruction;
  program->source_lines[program->count] = source_line;
  program->count++;
}

int add_constant(Program* program, ThuslyValue value) {
  append_constant(&program->constant_pool, value);

  return program->constant_pool.count - 1;
}
