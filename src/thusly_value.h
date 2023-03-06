#ifndef CTHUSLY_THUSLY_VALUE_H
#define CTHUSLY_THUSLY_VALUE_H

#include "common.h"

// Temporarily only handling doubles
typedef double ThuslyValue;

typedef struct {
  ThuslyValue* values;
  int count;
  int capacity;
} ConstantPool;

void init_constant_pool(ConstantPool* pool);
void free_constant_pool(ConstantPool* pool);
void append_constant(ConstantPool* pool, ThuslyValue value);
void print_value(ThuslyValue value);

#endif
