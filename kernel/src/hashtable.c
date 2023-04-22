#include <hashtable.h>
#include <stdlib.h>
#include <string.h>

hash_table *hash_table_create(size_t default_elsize)
{
  return hash_table_create_extra(
    default_elsize,
    0,
    HASH_TABLE_DEFAULT_MAX_COLLISIONS,
    HASH_TABLE_DEFAULT_INIT_BUCKETS
  );
}
hash_table *hash_table_create_extra(
  size_t default_elsize,
  hash_func_t hf,
  size_t max_collisions,
  size_t init_buckets
) {
  hash_table *ht = calloc(1, sizeof(hash_table));
  if(!ht)
    return 0;
  ht->nbuckets = init_buckets;
  ht->buckets = calloc(init_buckets, sizeof(ht_node *));
  if(!ht->buckets)
  {
    free(ht);
    return 0;
  }
  ht->default_elsize = default_elsize;
  ht->max_collisions = max_collisions;
  return ht;
}
void hash_table_destroy(hash_table *ht)
{
  for(size_t i = 0; i < ht->nbuckets; ++i)
  {
    ht_node *current = ht->buckets[i];
    while(current)
    {
      free(current);
      current = current->next;
    }
  }
  free(ht->buckets);
  free(ht);
}

void *hash_table_addkey(hash_table *ht, char const *key)
{
  return hash_table_addkey_extra(ht, key, ht->default_elsize);
}
void *hash_table_addkey_extra(hash_table *ht, char const *key, size_t elsize)
{
  size_t keylen = strlen(key);
  uint64_t hash = hash_table_calc_hash(ht, key);
  ht_node *newnode = calloc(1, sizeof(ht_node) + elsize + keylen + 1);
  ht_node **bucket = ht->buckets + hash % ht->nbuckets;
  if(!newnode)
    return 0;
  if((*bucket) && (*bucket)->len >= ht->max_collisions)
  {
    hash_table_resize_buckets(ht, 2*ht->nbuckets);
    bucket = ht->buckets + hash % ht->nbuckets;
  }
  newnode->next = *bucket;
  newnode->len = (*bucket) ? (*bucket)->len + 1 : 1;
  newnode->keyoff = sizeof(ht_node) + elsize;
  strcpy((void *) newnode + newnode->keyoff, key);
  *bucket = newnode;
  return (void *) (newnode+1);
}

void *hash_table_search(hash_table *ht, char const *key)
{
  uint64_t hash = hash_table_calc_hash(ht, key);
  ht_node *current = ht->buckets[hash % ht->nbuckets];
  while(current)
  {
    char const *ckey = (void *) current + current->keyoff;
    if(!strcmp(key, ckey))
      return (void *) (current+1);
    current = current->next;
  }
  return 0;
}

void hash_table_resize_buckets(hash_table *ht, size_t nnbuckets)
{
  if(!nnbuckets)
    return;
  ht_node **newbuckets = calloc(nnbuckets, sizeof(ht_node *));
  if(!newbuckets)
    return;
  for(size_t i = 0; i < ht->nbuckets; ++i)
  {
    ht_node *current = ht->buckets[i];
    while(current)
    {
      ht_node *next = current->next;
      char const *ckey = (void *) current + current->keyoff;
      uint64_t ckhash = hash_table_calc_hash(ht, ckey);
      ht_node **tbucket = newbuckets + ckhash % nnbuckets;
      current->next = *tbucket;
      *tbucket = current;
      current = next;
    }
  }
  free(ht->buckets);
  ht->nbuckets = nnbuckets;
  ht->buckets = newbuckets;
}

uint64_t hash_table_calc_hash(hash_table *ht, char const *key)
{
  if(ht->hf)
    return ht->hf(key);
  return hash_table_std_strhash(key);
}

uint64_t hash_table_std_strhash(char const *key)
{
  // from https://stackoverflow.com/a/57960443/21296545
  uint64_t h = 525201411107845655ull;
  while(*key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
    ++key;
  }
  return h;
}
