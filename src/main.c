#include "common.h"
#include "debug.h"
#include "program.h"

int main(int argc, const char* argv[]) {
  Program program;
  init_program(&program);

  // Temporary for debugging
  int constant_index = add_constant(&program, 20.5);
  append_instruction(&program, OP_CONSTANT, 1);
  append_instruction(&program, constant_index, 1);
  append_instruction(&program, OP_RETURN, 2);

  disassemble_program(&program, "Debugging");

  free_program(&program);

  return 1;
}
