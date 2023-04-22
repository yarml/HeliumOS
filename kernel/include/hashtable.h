#ifndef HELIUM_HASHTABLE_H
#define HELIUM_HASHTABLE_H

#include <stddef.h>
#include <stdint.h>

#define HASH_TABLE_DEFAULT_MAX_COLLISIONS (8)
#define HASH_TABLE_DEFAULT_INIT_BUCKETS   (32)

typedef struct HASH_TABLE hash_table;
typedef struct HT_NODE ht_node;
typedef uint64_t(*hash_func_t)(char const *key);

struct HASH_TABLE
{
  size_t nbuckets;
  ht_node **buckets;

  size_t max_collisions;
  size_t default_elsize;
  size_t length;

  hash_func_t hf;
};


struct HT_NODE
{
  ht_node *next;
  size_t keyoff;
  size_t len;
  size_t pad; // Size needs to be multiple of 16
};

hash_table *hash_table_create(size_t default_elsize);
hash_table *hash_table_create_extra(
  size_t default_elsize,
  hash_func_t hf,
  size_t max_collisions,
  size_t init_buckets
);
void hash_table_destroy(hash_table *ht);

void *hash_table_addkey(hash_table *ht, char const *key);
void *hash_table_addkey_extra(hash_table *ht, char const *key, size_t elsize);

void *hash_table_search(hash_table *ht, char const *key);

void hash_table_resize_buckets(hash_table *ht, size_t nnbuckets);

uint64_t hash_table_calc_hash(hash_table *ht, char const *key);

uint64_t hash_table_std_strhash(char const *key);

#endif