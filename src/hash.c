#include "libc_wrappers.h"
#include "hash.h"

#define EMPTY 0
#define TOMBSTONE 1
#define INUSE 2

struct hash_entry_t {
   hash_key_t key;
   hash_data_t data;
   hash_hashvalue_t hash_value;
   uint32_t status;
};

typedef struct hash_entry_t hash_entry_t;

int create_hashtable(hash_table_t *table, size_t initial_size, hash_func_t hashfunc, 
                     hash_cmp_t keycmp)
{
   hash_entry_t *newtable;
   int pagesize, entries_per_page;

   entries_per_page = gotcha_getpagesize() / sizeof(hash_entry_t);
   if (initial_size % entries_per_page)
      initial_size += entries_per_page - (initial_size % entries_per_page);

   newtable = (hash_entry_t *) gotcha_malloc(initial_size * sizeof(hash_entry_t));
   if (!newtable)
      return -1;

   table->table_size = initial_size;
   table->entry_count = 0;
   table->hashfunc = hashfunc;
   table->keycmp = keycmp;
   table->table = newtable;
   
   return 0;
}

static int insert(hash_table_t *table, hash_key_t key, hash_data_t data, hash_hashvalue_t value)
{
   hash_hashvalue_t index = value % table->table_size;
   hash_hashvalue_t startindex = index;

   do {
      hash_entry_t *entry = table->table + index;
      if (entry->status == EMPTY || entry->status == TOMBSTONE) {
         entry->key = key;
         entry->data = data;
         entry->hash_value = value;
         entry->status = INUSE;
         table->entry_count++;
         return 0;
      }
      index++;
      if (index == table->table_size)
         index = 0;
   } while (index != startindex);
   return -1;
}

int grow_hashtable(hash_table_t *table, size_t new_size)
{
   hash_table_t newtable;
   int result;
   size_t i;

   newtable.table_size = new_size;
   newtable.table = (hash_entry_t *) gotcha_malloc(new_size * sizeof(hash_entry_t));

   for (i = 0; i < table->table_size; i++) {
      if (table->table[i].status == EMPTY || table->table[i].status == TOMBSTONE)
         continue;
      result = insert(&newtable, table->table[i].key, table->table[i].data,
                      table->table[i].hash_value);
      if (result == -1) {
         return -1;
      }
   }

   destroy_hashtable(table);
   *table = newtable;
   return 0;
}

int destroy_hashtable(hash_table_t *table)
{
   gotcha_free(table->table);
   table->table_size = 0;
   table->entry_count = 0;
   table->hashfunc = NULL;
   table->keycmp = NULL;
   table->table = NULL;
   return 0;
}

static int lookup(hash_table_t *table, hash_key_t key, hash_entry_t **entry)
{
   size_t index, startindex;
   hash_hashvalue_t hashval;

   hashval = table->hashfunc(key);
   index = hashval % table->table_size;
   startindex = index;
   
   for (;;) {
      hash_entry_t *cur = table->table + index;
      if ((cur->status == INUSE) && 
          (cur->hash_value == hashval) && 
          (table->keycmp(cur->key, key) == 0)) {
         *entry = cur;
         return 0;
      }

      if (cur->status == EMPTY)
         return -1;
      index++;
      if (index == table->table_size)
         index = 0;
      if (index == startindex)
         return -1;
   }
}

int lookup_hashtable(hash_table_t *table, hash_key_t key, hash_data_t *data)
{
   hash_entry_t *entry;
   int result;

   result = lookup(table, key, &entry);
   if (result == -1)
      return -1;
   *data = entry->data;
   return 0;
}

int addto_hashtable(hash_table_t *table, hash_key_t key, hash_data_t data)
{
   size_t newsize, index, startindex;
   int result;
   hash_hashvalue_t val;

   newsize = table->table_size;
   while (table->entry_count > newsize/2)
      newsize *= 2;
   if (newsize != table->table_size) {
      result = grow_hashtable(table, newsize);
      if (result == -1)
         return -1;
   }

   val = table->hashfunc(key);
   index = val % table->table_size;
   startindex = index;

   for (;;) {
      hash_entry_t *entry = table->table + index;
      if (entry->status != INUSE) {
         entry->key = key;
         entry->data = data;
         entry->hash_value = val;
         entry->status = INUSE;
         table->entry_count++;
         return 0;
      }
      index++;
      if (index == table->table_size)
         index = 0;
      if (index == startindex)
         return -1;
   }
}

int removefrom_hashtable(hash_table_t *table, hash_key_t key)
{
   hash_entry_t *entry;
   int result;

   result = lookup(table, key, &entry);
   if (result == -1)
      return -1;

   entry->key = NULL;
   entry->data = NULL;
   entry->hash_value = 0;
   entry->status = TOMBSTONE;

   table->entry_count--;
   return 0;
}

hash_hashvalue_t strhash(const char *str)
{
   unsigned long hash = 5381;
   int c;

   while ((c = *str++))
      hash = hash * 33 + c;

   return (hash_hashvalue_t) hash;
}
