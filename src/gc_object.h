#ifndef CTHUSLY_GC_OBJECT_H
#define CTHUSLY_GC_OBJECT_H

#include "common.h"
#include "thusly_value.h"
#include "vm.h"

#define GET_GC_OBJECT_TYPE(thusly_value) (TO_C_OBJECT_PTR(thusly_value)->type)
#define IS_TEXT(thusly_value)            matches_gc_object_type(thusly_value, GC_OBJECT_TYPE_TEXT)

#define TO_TEXT(thusly_value)            ((TextObject*)TO_C_OBJECT_PTR(thusly_value))
#define TO_C_STRING(thusly_value)        (((TextObject*)TO_C_OBJECT_PTR(thusly_value))->chars)

typedef enum {
  GC_OBJECT_TYPE_TEXT,
} GCObjectType;

/// The common state of all dynamically (heap) allocated values (referred to as gc objects here).
///
/// IMPORTANT: All gc objects must have this `GCObject` struct as the first field so that a
/// pointer to `GCObject` or the enclosing struct (e.g `TextObject`) always points to the
/// first field of the `GCObject` struct. (There is never padding in the beginning of a struct.)
struct GCObject {
  GCObjectType type;
  // Pointer to the next heap-allocated object in the (intrusive) linked list.
  // (The head is pointed to by the VM's `Environment` struct.)
  struct GCObject* next;
};

/// A dynamically allocated text (string) value.
struct TextObject {
  // IMPORTANT: This field must be first (see notes in `GCObject`).
  GCObject base;
  char* chars;
  int length;
};

TextObject* claim_c_string(Environment* environment, char* chars, int length);
TextObject* copy_c_string(Environment* environment, const char* chars, int length);
void print_object(ThuslyValue value);

static inline bool matches_gc_object_type(ThuslyValue value, GCObjectType type) {
  return IS_GC_OBJECT(value) && TO_C_OBJECT_PTR(value)->type == type;
}

#endif
