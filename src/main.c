#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "exit_code.h"
#include "program.h"
#include "vm.h"

static void print_help() {
  fprintf(stderr, "Run in interactive mode: ./cthusly\nExecute a file: ./cthusly path/to/file\n");
}

static void run_repl(VM* vm) {
  char line[1024];
  while (true) {
    printf("> ");
    // Only handling single-line inputs.
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(vm, line);
  }
}

static char* read_file(const char* path) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "The file could not be opened (\"%s\").", path);
    exit(EXIT_CODE_IO_OP_ERROR);
  }

  // Going to the end of the file (`fseek()`) and checking how many bytes there are (`ftell()`)
  // determines the size of the source/string buffer needed when allocating it (`malloc()`).
  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  // Add +1 for terminating the string (null byte).
  char* source_buffer = (char*)malloc(file_size + 1);
  if (source_buffer == NULL) {
    fprintf(stderr, "There was not enough memory available to read the file (\"%s\").", path);
    exit(EXIT_CODE_IO_OP_ERROR);
  }

  // Read from the file and into the allocated string buffer.
  size_t num_bytes_read = fread(source_buffer, sizeof(char), file_size, file);
  if (num_bytes_read < file_size) {
    fprintf(stderr, "The file could not be read (\"%s\").", path);
    exit(EXIT_CODE_IO_OP_ERROR);
  }
  source_buffer[num_bytes_read] = '\0';

  fclose(file);

  return source_buffer;
}

static void run_file(VM* vm, const char* path) {
  char* source = read_file(path);
  ErrorReport report = interpret(vm, source);
  // `read_file()` uses `malloc()` for the source, thus it needs to be freed here.
  free(source);

  if (report == REPORT_COMPILE_ERROR)
    exit(EXIT_CODE_INPUT_DATA_ERROR);
  if (report == REPORT_RUNTIME_ERROR)
    exit(EXIT_CODE_INTERNAL_SOFTWARE_ERROR);
}

int main(int argc, const char* argv[]) {
  VM vm;
  init_vm(&vm);
  
  if (argc == 1)
    run_repl(&vm);
  else if (argc == 2)
    run_file(&vm, argv[1]);
  else {
    print_help();
    exit(EXIT_CODE_USAGE_ERROR);
  }

  free_vm(&vm);

  return EXIT_SUCCESS;
}
