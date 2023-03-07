#ifndef CTHUSLY_EXIT_CODE_H
#define CTHUSLY_EXIT_CODE_H

/// Program exit code (UNIX).
/// (see https://www.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+13.1-RELEASE&arch=default&format=html)
typedef enum {
  EXIT_CODE_USAGE_ERROR = 64,
  EXIT_CODE_INPUT_DATA_ERROR = 65,
  EXIT_CODE_INPUT_FILE_ERROR = 66,
  EXIT_CODE_INTERNAL_SOFTWARE_ERROR = 70,
  EXIT_CODE_IO_OP_ERROR = 74,
} ExitCode;

#endif
