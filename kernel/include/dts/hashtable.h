#ifndef HELIUM_DTS_HASHTABLE_H
#define HELIUM_DTS_HASHTABLE_H

#ifndef DTS_HASHTABLE_MAX_COLLISIONS
#define DTS_HASHTABLE_MAX_COLLISIONS (8)
#endif

#ifndef DTS_HASHTABLE_INIT_BUCKETS
#define DTS_HASHTABLE_INIT_BUCKETS (32)
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DTS_HASHTABLE      dts_hashtable;
typedef struct DTS_HASHTABLE_NODE dts_hashtable_node;

typedef size_t (*dts_keylen_f)(void const *key);
typedef size_t (*dts_hash_f)(void const *key);
typedef int (*dts_keycmp_f)(void const *k1, void const *k2);

typedef void (*dts_hashtable_destroy_pair_f)(
    dts_hashtable *ht, void const *k, void *obj
);

struct DTS_HASHTABLE_NODE {
  dts_hashtable_node *next;
  size_t              next_count;
  void               *key;
  void               *obj;
  size_t              objsize;
};

struct DTS_HASHTABLE {
  size_t               nbuckets;
  dts_hashtable_node **buckets;

  size_t max_collisions;
  size_t objsize;

  dts_keycmp_f keycmp;
  dts_hash_f   hash;
  dts_keylen_f keylen;
};

dts_hashtable *dts_hashtable_create_strkey(size_t esize);
dts_hashtable *dts_hashtable_create_uptrkey(size_t esize);
dts_hashtable *dts_hashtable_create(
    size_t       esize,  // esize 0 => no allocation for object, pointer is data
    size_t       max_collisions,
    size_t       init_nbuckets,
    dts_keycmp_f keycmp,
    dts_keylen_f
               keylen,  // Keylen NULL => no allocation for key, pointer is key
    dts_hash_f hash
);

void dts_hashtable_destroy(
    dts_hashtable *ht, dts_hashtable_destroy_pair_f destroy_pair
);

void *dts_hashtable_insert(dts_hashtable *ht, void const *key, void const *obj);
void *dts_hashtable_insert_extra(
    dts_hashtable *ht, void const *key, void const *obj, size_t objsize
);
void *dts_hashtable_search(dts_hashtable *ht, void const *key, bool *found);
void  dts_hashtable_remove(dts_hashtable *ht, void const *key, void *out_obj);

size_t dts_hashtable_std_strhash(void const *key);
size_t dts_hashtable_std_uptrhash(void const *key);

#endif