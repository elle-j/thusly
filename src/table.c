#include <stdio.h>
#include <string.h>

#include "gc_object.h"
#include "memory.h"
#include "table.h"

#define TABLE_MAX_LOAD 0.75
#define ENTRY_EXISTS(entry)   ((entry)->key != NULL)
#define TABLE_IS_EMPTY(table) ((table)->count == 0)

void table_init(Table* table) {
  table->entries = NULL;
  table->count = 0;
  table->capacity = 0;
}

void table_free(Table* table) {
  // -- TEMPORARY --
  #ifdef DEBUG_MODE
    if (flag_debug_execution)
      printf("FREEING TABLE..\n");
  #endif
  // ---------------

  FREE_ARRAY(TableEntry, table->entries, table->capacity);
  table_init(table);
}

static TextObject* find_interned_text(Table* interned_texts, const char* chars, int length, uint32_t hash_code) {
  uint32_t index = hash_code % interned_texts->capacity;
  while (true) {
    TableEntry* entry = &interned_texts->entries[index];
    if (!ENTRY_EXISTS(entry)) {
      bool is_tombstone = IS_BOOLEAN(entry->value);
      if (!is_tombstone)
        return NULL;
    }
    else if (
      entry->key->length == length &&
      entry->key->hash_code == hash_code &&
      memcmp(entry->key->chars, chars, length) == 0
    )
      return entry->key;

    index = (index + 1) % interned_texts->capacity;
  }
}

static TableEntry* find_new_or_existing_entry(TableEntry* entries, int capacity, TextObject* key) {
  // "Open addressing" using linear probing is used as the collision resolution method.
  uint32_t index = key->hash_code % capacity;
  TableEntry* next_available_entry = NULL;
  while (true) {
    TableEntry* entry = &entries[index];
    // `==` works for texts (strings) too since all texts are interned
    // (each text is unique and points to the same memory location).
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

static void entries_init(TableEntry* entries, int capacity) {
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = FROM_C_NULL;
  }
}

static void grow_and_rebuild_table(Table* table, int new_capacity) {
  TableEntry* new_entries = ALLOCATE(TableEntry, new_capacity);
  entries_init(new_entries, new_capacity);

  // Reset the count and increment it only if an entry exists in
  // order to not include tombstones in the count when rebuilding.
  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    TableEntry* old_entry = &table->entries[i];
    if (ENTRY_EXISTS(old_entry)) {
      TableEntry* new_entry = find_new_or_existing_entry(new_entries, new_capacity, old_entry->key);
      new_entry->key = old_entry->key;
      new_entry->value = old_entry->value;
      table->count++;
    }
  }

  FREE_ARRAY(TableEntry, table->entries, table->capacity);
  table->entries = new_entries;
  table->capacity = new_capacity;
}

bool table_get(Table* table, TextObject* key, ThuslyValue* out_value) {
  if (TABLE_IS_EMPTY(table))
    return false;

  TableEntry* entry = find_new_or_existing_entry(table->entries, table->capacity, key);
  bool exists = ENTRY_EXISTS(entry);
  if (exists)
    *out_value = entry->value;

  return exists;
}

TextObject* table_get_interned_text(Table* table, const char* chars, int length, uint32_t hash_code) {
  if (TABLE_IS_EMPTY(table))
    return NULL;

  return find_interned_text(table, chars, length, hash_code);
}

bool table_set(Table* table, TextObject* key, ThuslyValue value) {
  bool max_load_reached = table->count + 1 > table->capacity * TABLE_MAX_LOAD;
  if (max_load_reached)
    grow_and_rebuild_table(table, GROW_CAPACITY(table->capacity));

  TableEntry* entry = find_new_or_existing_entry(table->entries, table->capacity, key);
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

bool table_pop(Table* table, TextObject* key) {
  if (TABLE_IS_EMPTY(table))
    return false;

  TableEntry* entry = find_new_or_existing_entry(table->entries, table->capacity, key);
  bool exists = ENTRY_EXISTS(entry);
  if (exists)
    // Only place the tombstone, don't decrement the count (otherwise
    // `get_new_or_existing_entry()` can get stuck in an infinite loop
    // due to an incorrect (too small) load factor, causing the entries
    // array to not grow when needed).
    place_tombstone(entry);

  return exists;
}
