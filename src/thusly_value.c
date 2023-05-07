#include <stdio.h>
#include <string.h>

#include "gc_object.h"
#include "memory.h"
#include "thusly_value.h"

void constant_pool_init(ConstantPool* pool) {
  pool->values = NULL;
  pool->count = 0;
  pool->capacity = 0;
}

void constant_pool_free(ConstantPool* pool) {
  // -- TEMPORARY --
  #ifdef DEBUG_EXECUTION
    printf("FREEING CONSTANT POOL..\n");
  #endif
  // ---------------

  FREE_ARRAY(ThuslyValue, pool->values, pool->capacity);
  constant_pool_init(pool);
}

void constant_pool_add(ConstantPool* pool, ThuslyValue value) {
  bool max_capacity_reached = pool->count + 1 > pool->capacity;
  if (max_capacity_reached) {
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
    case TYPE_GC_OBJECT:
      return TO_C_OBJECT_PTR(a) == TO_C_OBJECT_PTR(b);
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
    case TYPE_GC_OBJECT:
      print_object(value);
      break;
  }
}
