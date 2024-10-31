#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gc_object.h"
#include "memory.h"

#define ALLOCATE_OBJECT(environment, type, gc_object_type) \
  (type*)allocate_object(environment, sizeof(type), gc_object_type)

/// Allocate memory for a `GCObject`.
///
/// The size of the object needs to be passed as an argument (rather than
/// using `sizeof(GCObject)`) since there are different-sized object types.
static GCObject* allocate_object(Environment* environment, size_t size, GCObjectType gc_object_type) {
  GCObject* object = (GCObject*)handle_reallocation(NULL, 0, size);
  object->type = gc_object_type;

  // Add the object to the linked list (inserting at the beginning).
  object->next = environment->gc_objects;
  environment->gc_objects = object;

  return object;
}

/// Allocate memory for a text object and add the object to the text intern pool.
static TextObject* allocate_text_object(Environment* environment, char* chars, int length, uint32_t hash_code) {
  TextObject* text = ALLOCATE_OBJECT(environment, TextObject, GC_OBJECT_TYPE_TEXT);
  text->chars = chars;
  text->length = length;
  text->hash_code = hash_code;
  table_set(&environment->texts, text, FROM_C_NULL);

  return text;
}

/// Produce a deterministic fixed-size hash code from a given key.
/// (Uses the FNV1 hash algorithm: http://www.isthe.com/chongo/tech/comp/fnv/)
static uint32_t hash(const char* key, int length) {
  #define FNV1_32_INIT ((uint32_t)2166136261u)
  #define FNV_32_PRIME ((uint32_t)16777619)

  uint32_t hash_code = FNV1_32_INIT;
  for (int i = 0; i < length; i++) {
    // Incorporate each char into the hash (xor the bottom with the current octet).
    hash_code ^= (uint8_t)key[i];
    // Shuffle the bits around (multiply by the 32 bit FNV magic prime).
    hash_code *= FNV_32_PRIME;
  }

  return hash_code;

  #undef FNV1_32_INIT
  #undef FNV_32_PRIME
}

/// Have a Thusly text object claim ownership of a C string.
TextObject* claim_c_string(Environment* environment, char* chars, int length) {
  uint32_t hash_code = hash(chars, length);
  TextObject* interned_text = table_get_interned_text(&environment->texts, chars, length, hash_code);
  if (interned_text != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned_text;
  }

  return allocate_text_object(environment, chars, length, hash_code);
}

/// Have a Thusly text object copy a C string.
TextObject* copy_c_string(Environment* environment, const char* chars, int length) {
  uint32_t hash_code = hash(chars, length);
  TextObject* interned_text = table_get_interned_text(&environment->texts, chars, length, hash_code);
  if (interned_text != NULL)
    return interned_text;

  // Allocate +1 for the terminating null byte.
  char* chars_copy = ALLOCATE(char, length + 1);
  memcpy(chars_copy, chars, length);
  chars_copy[length] = '\0';

  return allocate_text_object(environment, chars_copy, length, hash_code);
}

void print_object(ThuslyValue value) {
  switch (GET_GC_OBJECT_TYPE(value)) {
    case GC_OBJECT_TYPE_TEXT:
      printf("%s", TO_C_STRING(value));
      break;
  }
}
