#include "common.h"
#include "debug.h"
#include "program.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  VM vm;
  init_vm(&vm);
  Program program;
  init_program(&program);

  // Temporary for debugging
  int constant_index = add_constant(&program, 20.5);
  append_instruction(&program, OP_CONSTANT, 1);
  append_instruction(&program, constant_index, 1);
  append_instruction(&program, OP_RETURN, 2);

  disassemble_program(&program, "Debugging");

  interpret(&vm, &program);

  free_vm(&vm);
  free_program(&program);

  return 1;
}
