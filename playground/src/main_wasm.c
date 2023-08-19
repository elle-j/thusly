#include <stdio.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>

#include "../../src/common.h"
#include "../../src/exit_code.h"
#include "../../src/vm.h"

// If this was to be ported to C++, this macro is needed so that it treats it as an external C function.
// #ifdef __cplusplus
// #define EXTERN extern "C"
// #else
// #define EXTERN
// #endif

// Uncomment when removing `$interpreter_dir/main.c` from the `$files_to_compile`.
// bool flag_debug_compilation = false;
// bool flag_debug_execution = false;

// Using `EMSCRIPTEN_KEEPALIVE` before the function name prevents Emscripten-generated code
// from eliminating it as dead code. (It always just calls the `main()` function by default.)
/*EXTERN*/ EMSCRIPTEN_KEEPALIVE
int run_source(char* code, bool debug_compilation, bool debug_execution) {
  flag_debug_compilation = debug_compilation;
  flag_debug_execution = debug_execution;

  VM vm;
  vm_init(&vm);

  ErrorReport report = interpret(&vm, code);

  free(code);
  vm_free(&vm);

  // Must always end with newline!
  printf("\n");

  if (report == REPORT_COMPILE_ERROR)
    return EXIT_CODE_INPUT_DATA_ERROR;
  if (report == REPORT_RUNTIME_ERROR)
    return EXIT_CODE_INTERNAL_SOFTWARE_ERROR;

  return 0;
}

// Uncomment if not using `--no-entry` on Emscripten compilation and removing
// `$interpreter_dir/main.c` from the `$files_to_compile`.
// int main() {
//   printf("Hey, welcome to the Thusly playground!\n");
//   return EXIT_SUCCESS;
// }
