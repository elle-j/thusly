#include <stdlib.h>

#include "memory.h"
#include "object.h"

void* handle_reallocation(void* memory, size_t old_size, size_t new_size) {
  bool should_free = new_size == 0;
  if (should_free) {
    free(memory);
    return NULL;
  }

  // `realloc()` handles the cases where it should grow (new_size > old_size),
  // shrink (new_size < old_size), or allocate (old_size == 0). (It uses the
  // metadata of the memory block passed to know its size.)
  void* reallocated_memory = realloc(memory, new_size);
  if (reallocated_memory == NULL)
    // Not enough available memory.
    exit(1);

  return reallocated_memory;
}

static void free_object(Object* object) {
  switch (object->type) {
    case OBJECT_TYPE_TEXT: {
      TextObject* text = (TextObject*)object;
      FREE_ARRAY(char, text->chars, text->length + 1);
      FREE(TextObject, object);
      break;
    }
  }
}

void free_objects(Environment* environment) {
  Object* current = environment->objects;
  while (current != NULL) {
    Object* next = current->next;
    free_object(current);
    current = next;
  }
}
