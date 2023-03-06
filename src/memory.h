#ifndef CTHUSLY_MEMORY_H
#define CTHUSLY_MEMORY_H

#include "common.h"

#define GROWTH_FACTOR 2
#define MIN_GROWTH_THRESHOLD 10
#define GROW_CAPACITY(capacity) \
  ((capacity) < MIN_GROWTH_THRESHOLD ? MIN_GROWTH_THRESHOLD : (capacity) * GROWTH_FACTOR)

#define GROW_ARRAY(elem_type, array, old_capacity, new_capacity) \
  (elem_type*)reallocate(array, sizeof(elem_type) * (old_capacity), sizeof(elem_type) * (new_capacity))

#define FREE_ARRAY(elem_type, array, capacity) \
  reallocate(array, sizeof(elem_type) * (capacity), 0)

void* reallocate(void* memory, size_t old_capacity, size_t new_capacity);

#endif
