#ifndef CTHUSLY_DEBUG_H
#define CTHUSLY_DEBUG_H

#include "program.h"
#include "vm.h"

void disassemble_stack(VM* vm);
void disassemble_program(Program* program);
int disassemble_instruction(Program* program, int offset);
void disassembler_print_headings(const char* title);
void disassembler_indent_to_last_column();

#endif
