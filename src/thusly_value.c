#include <stdio.h>

#include "memory.h"
#include "thusly_value.h"

void init_constant_pool(ConstantPool* pool) {
  pool->values = NULL;
  pool->count = 0;
  pool->capacity = 0;
}

void free_constant_pool(ConstantPool* pool) {
  printf("FREEING CONSTANT POOL..\n"); // TEMPORARY

  FREE_ARRAY(ThuslyValue, pool->values, pool->capacity);
  init_constant_pool(pool);
}

void append_constant(ConstantPool* pool, ThuslyValue value) {
  bool should_grow = pool->capacity < pool->count + 1;
  if (should_grow) {
    int old_capacity = pool->capacity;
    pool->capacity = GROW_CAPACITY(old_capacity);
    pool->values = GROW_ARRAY(ThuslyValue, pool->values, old_capacity, pool->capacity);
  }

  pool->values[pool->count] = value;
  pool->count++;
}

bool values_are_equal(ThuslyValue a, ThuslyValue b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
    case TYPE_BOOLEAN:
      return TO_C_BOOL(a) == TO_C_BOOL(b);
    case TYPE_NONE:
      return true;
    case TYPE_NUMBER:
      return TO_C_DOUBLE(a) == TO_C_DOUBLE(b);
    default:
      // This should not be reachable.
      return false;
  }
}

void print_value(ThuslyValue value) {
  switch (value.type) {
    case TYPE_BOOLEAN:
      printf(TO_C_BOOL(value) ? "true" : "false");
      break;
    case TYPE_NONE:
      printf("none");
      break;
    case TYPE_NUMBER:
      printf("%g", TO_C_DOUBLE(value));
      break;
  }
}
