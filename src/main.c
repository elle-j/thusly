#include <stdio.h>

#include "common.h"
#include "program.h"

int main(int argc, const char* argv[]) {
  printf("MAIN..\n"); // TEMPORARY

  Program program;
  init_program(&program);
  append_instruction(&program, OP_RETURN);
  free_program(&program);

  return 1;
}
