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
  TableEntry* next_available_entry = NULL;
  while (true) {
    TableEntry* entry = &entries[index];
    // TODO: Replace `==` key equality
    if (entry->key == key)
      return entry;
    
    if (!ENTRY_EXISTS(entry)) {
      bool is_tombstone = IS_BOOLEAN(entry->value);
      if (is_tombstone)
        next_available_entry = next_available_entry ? next_available_entry : entry;
      else
        return next_available_entry ? next_available_entry : entry;
    }

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

  // Reset the count and increment it only if an entry exists in
  // order to not include tombstones in the count when rebuilding.
  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    TableEntry* old_entry = &table->entries[i];
    if (ENTRY_EXISTS(old_entry)) {
      TableEntry* new_entry = get_new_or_existing_entry(new_entries, new_capacity, old_entry->key);
      new_entry->key = old_entry->key;
      new_entry->value = old_entry->value;
      table->count++;
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
  bool is_brand_new = !exists && IS_NONE(entry->value); // Tombstones are not counted as "brand new".
  if (is_brand_new)
    table->count++;

  entry->key = key;
  entry->value = value;

  return !exists;
}

static void place_tombstone(TableEntry* entry) {
  entry->key = NULL;
  entry->value = FROM_C_BOOL(true);
}

bool pop_table(Table* table, TextObject* key) {
  if (table->count == 0)
    return false;

  TableEntry* entry = get_new_or_existing_entry(table->entries, table->capacity, key);
  bool exists = ENTRY_EXISTS(entry);
  if (exists)
    // Only place the tombstone, don't decrement the count (otherwise
    // `get_new_or_existing_entry()` can get stuck in an infinite loop
    // due to an incorrect (too small) load factor, causing the entries
    // array to not grow when needed).
    place_tombstone(entry);

  return exists;
}
