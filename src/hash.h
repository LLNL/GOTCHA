#if !defined(HASH_H_)
#define HASH_H_

#include <stdlib.h>
#include <stdint.h>

typedef void* hash_key_t;
typedef void* hash_data_t;
typedef int hash_hashvalue_t;
typedef hash_hashvalue_t (*hash_func_t)(hash_data_t data);
typedef int (*hash_cmp_t)(hash_key_t a, hash_key_t b);

struct hash_entry_t;

typedef struct 
{
   size_t table_size;
   size_t entry_count;
   hash_func_t hashfunc;
   hash_cmp_t keycmp;
   struct hash_entry_t *table;
} hash_table_t;

int create_hashtable(hash_table_t *table, size_t initial_size, hash_func_t func, 
                     hash_cmp_t keycmp);
int grow_hashtable(hash_table_t *table, size_t new_size);
int destroy_hashtable(hash_table_t *table);

int lookup_hashtable(hash_table_t *table, hash_key_t key, hash_data_t *data);
int addto_hashtable(hash_table_t *table, hash_key_t key, hash_data_t data);
int removefrom_hashtable(hash_table_t *table, hash_key_t key);

hash_hashvalue_t strhash(const char *str);

#endif
