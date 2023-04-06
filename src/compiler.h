#ifndef CTHUSLY_COMPILER_H
#define CTHUSLY_COMPILER_H

#include "program.h"
#include "vm.h"

bool compile(Environment* environment, const char* source, Program* out_program);

#endif
