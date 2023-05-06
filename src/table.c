#include "memory.h"
#include "table.h"

void init_table(Table* table) {
  table->entries = NULL;
  table->count = 0;
  table->capacity = 0;
}

void free_table(Table* table) {
  FREE_ARRAY(TableEntry, table->entries, table->capacity);
  init_table(table);
}
