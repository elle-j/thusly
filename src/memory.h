#ifndef CTHUSLY_MEMORY_H
#define CTHUSLY_MEMORY_H

#include "common.h"
#include "vm.h"

#define ALLOCATE(type, capacity) (type*)handle_reallocation(NULL, 0, sizeof(type) * (capacity))

#define GROWTH_FACTOR 2
#define MIN_GROWTH_THRESHOLD 10
#define GROW_CAPACITY(capacity) \
  ((capacity) < MIN_GROWTH_THRESHOLD ? MIN_GROWTH_THRESHOLD : (capacity) * GROWTH_FACTOR)

#define GROW_ARRAY(elem_type, array, old_capacity, new_capacity) \
  (elem_type*)handle_reallocation(array, sizeof(elem_type) * (old_capacity), sizeof(elem_type) * (new_capacity))

#define FREE_ARRAY(elem_type, array, capacity) \
  handle_reallocation(array, sizeof(elem_type) * (capacity), 0)

#define FREE(type, memory) handle_reallocation(memory, sizeof(type), 0)

void* handle_reallocation(void* memory, size_t old_capacity, size_t new_capacity);
void free_objects(Environment* environment);

#endif
