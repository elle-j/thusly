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

void table_init(Table* table);
void table_free(Table* table);
bool table_get(Table* table, TextObject* key, ThuslyValue* out_value);
bool table_set(Table* table, TextObject* key, ThuslyValue value);
bool table_pop(Table* table, TextObject* key);
TextObject* table_get_interned_text(Table* table, const char* chars, int length, uint32_t hash_code);

#endif
