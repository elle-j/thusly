#ifndef CTHUSLY_DEBUG_H
#define CTHUSLY_DEBUG_H

#include "program.h"
#include "vm.h"

void disassemble_stack(VM* vm);
void disassemble_program(Program* program, const char* name);
int disassemble_instruction(Program* program, int offset);

#endif
