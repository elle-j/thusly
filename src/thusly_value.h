#ifndef CTHUSLY_THUSLY_VALUE_H
#define CTHUSLY_THUSLY_VALUE_H

#include "common.h"

typedef struct Object Object;
typedef struct TextObject TextObject;

/// Built-in data types for a ThuslyValue.
typedef enum {
  TYPE_BOOLEAN,
  TYPE_NONE,
  TYPE_NUMBER,
  // Dynamically (heap) allocated type (e.g. `text` (string))
  TYPE_OBJECT,
} DataType;

typedef struct {
  DataType type;
  // A ThuslyValue can hold either of these underlying C types.
  union {
    bool c_bool;
    double c_double;
    Object* c_object_ptr;
  } to;
} ThuslyValue;

#define IS_BOOLEAN(thusly_value)      ((thusly_value).type == TYPE_BOOLEAN)
#define IS_NONE(thusly_value)         ((thusly_value).type == TYPE_NONE)
#define IS_NUMBER(thusly_value)       ((thusly_value).type == TYPE_NUMBER)
#define IS_OBJECT(thusly_value)       ((thusly_value).type == TYPE_OBJECT)

#define TO_C_BOOL(thusly_value)       ((thusly_value).to.c_bool)
#define TO_C_DOUBLE(thusly_value)     ((thusly_value).to.c_double)
#define TO_C_OBJECT_PTR(thusly_value) ((thusly_value).to.c_object_ptr)

#define FROM_C_BOOL(c_value)          ((ThuslyValue){ TYPE_BOOLEAN, { .c_bool = c_value } })
#define FROM_C_NULL                   ((ThuslyValue){ TYPE_NONE, { .c_double = 0 } })
#define FROM_C_DOUBLE(c_value)        ((ThuslyValue){ TYPE_NUMBER, { .c_double = c_value } })
#define FROM_C_OBJECT_PTR(c_ptr)      ((ThuslyValue){ TYPE_OBJECT, { .c_object_ptr = (Object*)c_ptr } })

typedef struct {
  ThuslyValue* values;
  int count;
  int capacity;
} ConstantPool;

void init_constant_pool(ConstantPool* pool);
void free_constant_pool(ConstantPool* pool);
void append_constant(ConstantPool* pool, ThuslyValue value);
bool values_are_equal(ThuslyValue a, ThuslyValue b);
void print_value(ThuslyValue value);

#endif
