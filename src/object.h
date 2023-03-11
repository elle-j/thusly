#ifndef CTHUSLY_OBJECT_H
#define CTHUSLY_OBJECT_H

#include "common.h"
#include "thusly_value.h"

#define GET_OBJECT_TYPE(thusly_value) (TO_C_OBJECT_PTR(thusly_value)->type)
#define IS_TEXT(thusly_value)         is_object_type(thusly_value, OBJECT_TYPE_TEXT)

#define TO_TEXT(thusly_value)         ((TextObject*)TO_C_OBJECT_PTR(thusly_value))
#define TO_C_STRING(thusly_value)     (((TextObject*)TO_C_OBJECT_PTR(thusly_value))->chars)

typedef enum {
  OBJECT_TYPE_TEXT,
} ObjectType;

/// The common state of all dynamically (heap) allocated values (referred to as objects).
///
/// IMPORTANT: All objects must have this `Object` struct as the first field so that a
/// pointer to `Object` or the enclosing struct (e.g `TextObject`) always points to the
/// first field of the `Object` struct. (There is never padding in the beginning of a struct.)
struct Object {
  ObjectType type;
};

/// A dynamically allocated text (string) value.
struct TextObject {
  // IMPORTANT: This field must be first (see notes in `Object`).
  Object base;
  int length;
  char* chars;
};

static inline bool is_object_type(ThuslyValue value, ObjectType type) {
  return IS_OBJECT(value) && TO_C_OBJECT_PTR(value)->type == type;
}

#endif
