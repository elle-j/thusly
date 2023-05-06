#include <stddef.h>
#include <stdint.h>
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

static TextObject* allocate_text_object(Environment* environment, char* chars, int length, uint32_t hash_code) {
  TextObject* text = ALLOCATE_OBJECT(environment, TextObject, GC_OBJECT_TYPE_TEXT);
  text->chars = chars;
  text->length = length;
  text->hash_code = hash_code;

  return text;
}

static uint32_t hash(const char* key, int length) {
  // FNV1 hash algorithm: http://www.isthe.com/chongo/tech/comp/fnv/
  #define FNV1_32_INIT 2166136261u;
  #define FNV_32_PRIME 16777619;

  uint32_t hash = FNV1_32_INIT;
  for (int i = 0; i < length; i++) {
    // Incorporate each char into the hash (xor the bottom with the current octet).
    hash ^= (uint8_t)key[i];
    // Shuffle the bits (multiply by the 32 bit FNV magic prime)
    hash *= FNV_32_PRIME;
  }

  return hash;

  #undef FNV1_32_INIT
  #undef FNV_32_PRIME
}

TextObject* claim_c_string(Environment* environment, char* chars, int length) {
  return allocate_text_object(environment, chars, length, hash(chars, length));
}

TextObject* copy_c_string(Environment* environment, const char* chars, int length) {
  // Allocate +1 for the terminating null byte.
  char* chars_copy = ALLOCATE(char, length + 1);
  memcpy(chars_copy, chars, length);
  chars_copy[length] = '\0';

  return allocate_text_object(environment, chars_copy, length, hash(chars, length));
}

void print_object(ThuslyValue value) {
  switch (GET_GC_OBJECT_TYPE(value)) {
    case GC_OBJECT_TYPE_TEXT:
      printf("%s", TO_C_STRING(value));
      break;
  }
}
