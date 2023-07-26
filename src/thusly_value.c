#include <stdio.h>
#include <string.h>

#include "gc_object.h"
#include "thusly_value.h"

bool values_are_equal(ThuslyValue a, ThuslyValue b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
    case TYPE_BOOLEAN:
      return TO_C_BOOL(a) == TO_C_BOOL(b);
    case TYPE_NONE:
      return true;
    case TYPE_NUMBER:
      return TO_C_DOUBLE(a) == TO_C_DOUBLE(b);
    case TYPE_GC_OBJECT:
      return TO_C_OBJECT_PTR(a) == TO_C_OBJECT_PTR(b);
    default:
      // This should not be reachable.
      return false;
  }
}

void print_value(ThuslyValue value) {
  switch (value.type) {
    case TYPE_BOOLEAN:
      printf(TO_C_BOOL(value) ? "true" : "false");
      break;
    case TYPE_NONE:
      printf("none");
      break;
    case TYPE_NUMBER:
      printf("%g", TO_C_DOUBLE(value));
      break;
    case TYPE_GC_OBJECT:
      print_object(value);
      break;
  }
}
