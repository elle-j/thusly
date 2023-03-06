#include <stdlib.h>

#include "memory.h"

void* reallocate(void* memory, size_t old_size, size_t new_size) {
  bool should_free = new_size == 0;
  if (should_free) {
    free(memory);
    return NULL;
  }

  // `realloc()` handles the cases where we should grow (new_size > old_size),
  // shrink (new_size < old_size), or allocate (old_size == 0). (It uses the
  // metadata of the memory block passed to know it's size.)
  void* reallocated_memory = realloc(memory, new_size);
  if (reallocated_memory == NULL)
    // Not enough available memory.
    exit(1);

  return reallocated_memory;
}
