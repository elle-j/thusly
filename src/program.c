#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "program.h"

void constant_pool_init(ConstantPool* pool) {
  pool->values = NULL;
  pool->count = 0;
  pool->capacity = 0;
}

void constant_pool_free(ConstantPool* pool) {
  // -- TEMPORARY --
  #ifdef DEBUG_EXECUTION
    printf("FREEING CONSTANT POOL..\n");
  #endif
  // ---------------

  FREE_ARRAY(ThuslyValue, pool->values, pool->capacity);
  constant_pool_init(pool);
}

void constant_pool_add(ConstantPool* pool, ThuslyValue value) {
  bool max_capacity_reached = pool->count + 1 > pool->capacity;
  if (max_capacity_reached) {
    int old_capacity = pool->capacity;
    pool->capacity = GROW_CAPACITY(old_capacity);
    pool->values = GROW_ARRAY(ThuslyValue, pool->values, old_capacity, pool->capacity);
  }

  pool->values[pool->count] = value;
  pool->count++;
}

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
  bool max_capacity_reached = program->count + 1 > program->capacity;
  if (max_capacity_reached) {
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
