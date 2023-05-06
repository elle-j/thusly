#ifndef CTHUSLY_TABLE_H
#define CTHUSLY_TABLE_H

#include "thusly_value.h"

/// A key-value entry pair in a hash table.
typedef struct {
  // Only text types are supported as keys.
  TextObject* key;
  ThuslyValue value;
} TableEntry;

/// A hash table for looking up values through names.
typedef struct {
  TableEntry* entries;
  int count;
  int capacity;
} Table;

void init_table(Table* table);
void free_table(Table* table);

#endif
