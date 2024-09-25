#include <stdio.h>
#include <stdlib.h>

#include "gc_object.h"
#include "memory.h"

/// Handle reallocation (allocating, freeing, growing, shrinking) of memory.
/// This manages all dynamic memory.
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
    exit(EXIT_FAILURE);

  return reallocated_memory;
}

static void free_object(GCObject* object) {
  switch (object->type) {
    case GC_OBJECT_TYPE_TEXT: {
      TextObject* text = (TextObject*)object;
      FREE_ARRAY(char, text->chars, text->length + 1);
      FREE(TextObject, object);
      break;
    }
  }
}

void free_objects(Environment* environment) {
  // -- TEMPORARY --
  #ifdef DEBUG_MODE
    if (flag_debug_execution)
      printf("FREEING GC OBJECTS..\n");
  #endif
  // ---------------

  GCObject* current = environment->gc_objects;
  while (current != NULL) {
    GCObject* next = current->next;
    free_object(current);
    current = next;
  }
}
