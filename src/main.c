#include "common.h"
#include "debug.h"
#include "program.h"

int main(int argc, const char* argv[]) {
  Program program;
  init_program(&program);
  append_instruction(&program, OP_RETURN);

  disassemble_program(&program, "Some name");
  free_program(&program);

  return 1;
}
