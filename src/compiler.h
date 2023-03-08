#ifndef CTHUSLY_COMPILER_H
#define CTHUSLY_COMPILER_H

#include "program.h"
#include "vm.h"

bool compile(VM* vm, const char* source, Program* out_program);

#endif
