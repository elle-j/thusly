#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "gc_object.h"
#include "memory.h"

#define ALLOCATE_OBJECT(environment, type, gc_object_type) \
  (type*)allocate_object(environment, sizeof(type), gc_object_type)

static GCObject* allocate_object(Environment* environment, size_t size, GCObjectType gc_object_type) {
  // `size` needs to come from the argument (rather than using `sizeof(GCObject)`)
  // since there are different-sized object types.
  GCObject* object = (GCObject*)handle_reallocation(NULL, 0, size);
  object->type = gc_object_type;

  // Add the object to the linked list (inserting at the beginning).
  object->next = environment->gc_objects;
  environment->gc_objects = object;

  return object;
}

static TextObject* allocate_text_object(Environment* environment, char* chars, int length) {
  TextObject* text = ALLOCATE_OBJECT(environment, TextObject, GC_OBJECT_TYPE_TEXT);
  text->chars = chars;
  text->length = length;

  return text;
}

TextObject* claim_c_string(Environment* environment, char* chars, int length) {
  return allocate_text_object(environment, chars, length);
}

TextObject* copy_c_string(Environment* environment, const char* chars, int length) {
  // Allocate +1 for the terminating null byte.
  char* chars_copy = ALLOCATE(char, length + 1);
  memcpy(chars_copy, chars, length);
  chars_copy[length] = '\0';

  return allocate_text_object(environment, chars_copy, length);
}

void print_object(ThuslyValue value) {
  switch (GET_GC_OBJECT_TYPE(value)) {
    case GC_OBJECT_TYPE_TEXT:
      printf("%s", TO_C_STRING(value));
      break;
  }
}
