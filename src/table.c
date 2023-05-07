#include "gc_object.h"
#include "memory.h"
#include "table.h"

#define TABLE_MAX_LOAD 0.75

#define ENTRY_EXISTS(entry) ((entry)->key != NULL)

void init_table(Table* table) {
  table->entries = NULL;
  table->count = 0;
  table->capacity = 0;
}

void free_table(Table* table) {
  FREE_ARRAY(TableEntry, table->entries, table->capacity);
  init_table(table);
}

static TableEntry* get_new_or_existing_entry(TableEntry* entries, int capacity, TextObject* key) {
  uint32_t index = key->hash_code % capacity;
  while (true) {
    TableEntry* entry = &entries[index];
    // TODO: Replace `==` key equality
    if (entry->key == key || !ENTRY_EXISTS(entry))
      return entry;

    index = (index + 1) % capacity;
  }
}

static void init_entries(TableEntry* entries, int capacity) {
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = FROM_C_NULL;
  }
}

static void rebuild_table(Table* table, int new_capacity) {
  TableEntry* new_entries = ALLOCATE(TableEntry, new_capacity);
  init_entries(new_entries, new_capacity);

  for (int i = 0; i < table->capacity; i++) {
    TableEntry* old_entry = &table->entries[i];
    if (ENTRY_EXISTS(old_entry)) {
      TableEntry* new_entry = get_new_or_existing_entry(new_entries, new_capacity, old_entry->key);
      new_entry->key = old_entry->key;
      new_entry->value = old_entry->value;
    }
  }

  FREE_ARRAY(TableEntry, table->entries, table->capacity);
  table->entries = new_entries;
  table->capacity = new_capacity;
}

bool get_table(Table* table, TextObject* key, ThuslyValue* out_value) {
  if (table->count == 0)
    return false;

  TableEntry* entry = get_new_or_existing_entry(table->entries, table->capacity, key);
  bool exists = ENTRY_EXISTS(entry);
  if (exists)
    *out_value = entry->value;

  return exists;
}

bool set_table(Table* table, TextObject* key, ThuslyValue value) {
  bool should_grow = table->count + 1 > table->capacity * TABLE_MAX_LOAD;
  if (should_grow)
    rebuild_table(table, GROW_CAPACITY(table->capacity));

  TableEntry* entry = get_new_or_existing_entry(table->entries, table->capacity, key);
  bool exists = ENTRY_EXISTS(entry);
  if (!exists)
    table->count++;

  entry->key = key;
  entry->value = value;

  return !exists;
}
