#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "exit_code.h"
#include "program.h"
#include "vm.h"

static void print_help(FILE* fout) {
  fprintf(fout,
    "\n"
    "Usage: ./bin/cthusly [options] [path]\n"
    "\n"
    "    The REPL (interactive prompt) starts if no [path] is provided\n"
    "\n"
    "    -h, --help                Show usage\n"
    "\n"
  );
}

static void run_repl() {
  VM vm;
  vm_init(&vm);

  char line[1024];
  while (true) {
    printf("> ");
    // Only handling single-line inputs.
    // TODO: Provide alternative grammar for handling NEWLINE and single inputs.
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(&vm, line);
  }

  vm_free(&vm);
}

static char* read_file(const char* path) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "The file could not be opened. (File name: \"%s\")\n", path);
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
    fprintf(stderr, "There was not enough memory available to read the file (\"%s\").\n", path);
    exit(EXIT_CODE_IO_OP_ERROR);
  }

  // Read from the file and into the allocated string buffer.
  size_t num_bytes_read = fread(source_buffer, sizeof(char), file_size, file);
  if (num_bytes_read < file_size) {
    fprintf(stderr, "The file could not be read (\"%s\").\n", path);
    exit(EXIT_CODE_IO_OP_ERROR);
  }
  source_buffer[num_bytes_read] = '\0';

  fclose(file);

  return source_buffer;
}

static void run_file(const char* path) {
  VM vm;
  vm_init(&vm);
  char* source = read_file(path);

  ErrorReport report = interpret(&vm, source);

  // `read_file()` uses `malloc()` for the source, thus it needs to be freed here.
  free(source);
  vm_free(&vm);

  if (report == REPORT_COMPILE_ERROR)
    exit(EXIT_CODE_INPUT_DATA_ERROR);
  if (report == REPORT_RUNTIME_ERROR)
    exit(EXIT_CODE_INTERNAL_SOFTWARE_ERROR);
}

int main(int argc, const char* argv[]) {
  if (argc == 1)
    run_repl();
  else if (argc == 2) {
    const char* argv1 = argv[1];
    if (strcmp(argv1, "-h") == 0 || strcmp(argv1, "--help") == 0)
      print_help(stdout);
    else
      run_file(argv1);
  }
  /*else if (argc == 3) {
    // TODO:
    // Currently only allowing '-h' or '--help' without providing the [path] as
    // the next arg. Other flags, e.g. debug flags, will be allowed as the arg
    // preceding the [path]. Add to this block when 3 args are supported.
  }*/
  else {
    print_help(stderr);
    return EXIT_CODE_USAGE_ERROR;
  }

  return EXIT_SUCCESS;
}
