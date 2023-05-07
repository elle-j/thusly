#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "program.h"

void program_init(Program* program) {
  program->source_lines = NULL;
  program->instructions = NULL;
  program->count = 0;
  program->capacity = 0;
  constant_pool_init(&program->constant_pool);
}

void program_free(Program* program) {
  // -- TEMPORARY --
  #ifdef DEBUG_EXECUTION
    printf("FREEING PROGRAM..\n");
  #endif
  // ---------------

  FREE_ARRAY(int, program->source_lines, program->capacity);
  FREE_ARRAY(byte, program->instructions, program->capacity);
  constant_pool_free(&program->constant_pool);
  program_init(program);
}

void program_write(Program* program, byte instruction, int source_line) {
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

int program_add_constant(Program* program, ThuslyValue value) {
  constant_pool_add(&program->constant_pool, value);

  return program->constant_pool.count - 1;
}
