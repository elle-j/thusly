#include <stdio.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>

// If this was to be ported to C++, this macro is needed so that it treats it as an external C function.
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

// Using `EMSCRIPTEN_KEEPALIVE` before the function name prevents Emscripten-generated code
// from eliminating it as dead code. (It always just calls the `main()` function by default.)
EMSCRIPTEN_KEEPALIVE
int run_source(char* code) {
  // Must always end with newline!
  printf("'run_source()' called with:\n%s\n", code);
  free(code);

  return 0;
}

int main() {
  printf("Hey, welcome to the Thusly playground!\n");
  return 0;
}
