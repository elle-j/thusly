#ifndef CTHUSLY_COMMON_H
#define CTHUSLY_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// Whether to support debug mode.
/// Comment/uncomment to disable/enable support for debug flags.
/// (Whether to print output is still controlled via the flags.)
#define DEBUG_MODE

/// Flag used by the implementer to output temporary information
/// when debugging.
// #define DEBUG_MODE_IMPLEMENTER

extern bool flag_debug_compilation;
extern bool flag_debug_execution;

typedef uint8_t byte;

#endif
