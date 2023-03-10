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

void print_value(ThuslyValue value) {
  // TODO: Temporarily assuming it's a double.
  printf("%g", TO_C_DOUBLE(value));
}
