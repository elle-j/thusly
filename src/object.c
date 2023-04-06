#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"

#define ALLOCATE_OBJECT(environment, type, object_type) \
  (type*)allocate_object(environment, sizeof(type), object_type)

static Object* allocate_object(Environment* environment, size_t size, ObjectType object_type) {
  // `size` needs to come from the argument (rather than using `sizeof(Object)`)
  // since there are different-sized object types.
  Object* object = (Object*)handle_reallocation(NULL, 0, size);
  object->type = object_type;

  // Add the object to the linked list (inserting at the beginning).
  object->next = environment->objects;
  environment->objects = object;

  return object;
}

static TextObject* allocate_text_object(Environment* environment, char* chars, int length) {
  TextObject* text = ALLOCATE_OBJECT(environment, TextObject, OBJECT_TYPE_TEXT);
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
  switch (GET_OBJECT_TYPE(value)) {
    case OBJECT_TYPE_TEXT:
      printf("%s", TO_C_STRING(value));
      break;
  }
}
