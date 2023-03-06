#include <stdlib.h>
#include <stdio.h>

#include "program.h"

void init_program(Program* program) {
  printf("INIT PROGRAM..\n"); // TEMPORARY

  program->instructions = NULL;
  program->count = 0;
  program->capacity = 0;
}

void free_program(Program* program) {
  printf("FREEING PROGRAM..\n"); // TEMPORARY
  
  FREE_ARRAY(byte, program->instructions, program->capacity);
  init_program(program);
}

void append_instruction(Program* program, byte instruction) {
  printf("APPENDING INSTRUCTION..\n"); // TEMPORARY

  if (program->capacity < program->count + 1) {
    int old_capacity = program->capacity;
    program->capacity = GROW_CAPACITY(old_capacity);
    program->instructions = GROW_ARRAY(byte, program->instructions, old_capacity, program->capacity);
  }

  program->instructions[program->count] = instruction;
  program->count++;
}
