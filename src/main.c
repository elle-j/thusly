#include "common.h"
#include "debug.h"
#include "program.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  VM vm;
  init_vm(&vm);
  Program program;
  init_program(&program);

  // Temporary for debugging:
  // -((20.5 + 11.5) / 4)
  // Expected behavior (no precedence, only left-to-right at the moment):
  // 1) 20.5 + 11.5 = 32
  // 2) 32 / 4 = 8
  // 3) -(8) = -8
  // 4) -8
  int constant_index = add_constant(&program, 20.5);
  append_instruction(&program, OP_CONSTANT, 1);
  append_instruction(&program, constant_index, 1);

  constant_index = add_constant(&program, 11.5);
  append_instruction(&program, OP_CONSTANT, 1);
  append_instruction(&program, constant_index, 1);

  append_instruction(&program, OP_ADD, 1);

  constant_index = add_constant(&program, 4);
  append_instruction(&program, OP_CONSTANT, 1);
  append_instruction(&program, constant_index, 1);

  append_instruction(&program, OP_DIVIDE, 1);

  append_instruction(&program, OP_NEGATE, 1);

  append_instruction(&program, OP_RETURN, 2);

  disassemble_program(&program, "Program");

  interpret(&vm, &program);

  free_vm(&vm);
  free_program(&program);

  return 1;
}
