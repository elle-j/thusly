#ifndef CTHUSLY_DEBUG_H
#define CTHUSLY_DEBUG_H

#include "program.h"

void disassemble_program(Program* program, const char* name);
int disassemble_instruction(Program* program, int offset);

#endif
