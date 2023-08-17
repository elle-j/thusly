#ifndef CTHUSLY_COMMON_H
#define CTHUSLY_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Comment/uncomment to disable/enable debug output
// #define DEBUG_COMPILATION
// #define DEBUG_EXECUTION

// Comment/uncomment to disable/enable debug mode.
// (Whether to print output is still controlled via the flags.)
#define DEBUG_MODE

#ifdef DEBUG_MODE
extern bool flag_debug_compilation;
extern bool flag_debug_execution;
#endif

typedef uint8_t byte;

#endif
