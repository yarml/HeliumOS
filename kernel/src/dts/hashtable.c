#include <stdlib.h>
#include <string.h>

#include <dts/hashtable.h>

static int uptr_cmp(void const *k1, void const *k2) {
  return (uintptr_t)k1 - (uintptr_t)k2;
}

dts_hashtable *dts_hashtable_create_strkey(size_t esize) {
  return dts_hashtable_create(
      esize,
      DTS_HASHTABLE_MAX_COLLISIONS,
      DTS_HASHTABLE_INIT_BUCKETS,
      (dts_keycmp_f)strcmp,
      (dts_keylen_f)strlen,
      dts_hashtable_std_strhash
  );
}
dts_hashtable *dts_hashtable_create_uptrkey(size_t esize) {
  return dts_hashtable_create(
      esize,
      DTS_HASHTABLE_MAX_COLLISIONS,
      DTS_HASHTABLE_INIT_BUCKETS,
      uptr_cmp,
      NULL,
      dts_hashtable_std_uptrhash
  );
}

dts_hashtable *dts_hashtable_create(
    size_t       esize,
    size_t       max_collisions,
    size_t       init_nbuckets,
    dts_keycmp_f keycmp,
    dts_keylen_f keylen,
    dts_hash_f   hash
) {
  dts_hashtable *ht = calloc(1, sizeof(dts_hashtable));
  if (!ht) {
    return 0;
  }

  ht->nbuckets = init_nbuckets;
  ht->buckets  = calloc(init_nbuckets, sizeof(dts_hashtable_node *));
  if (!ht->buckets) {
    free(ht);
    return 0;
  }

  ht->objsize        = esize;
  ht->max_collisions = max_collisions;

  ht->keycmp = keycmp;
  ht->keylen = keylen;
  ht->hash   = hash;

  return ht;
}

void dts_hashtable_destroy(
    dts_hashtable *ht, dts_hashtable_destroy_pair_f destroy_pair
) {
  for (size_t i = 0; i < ht->nbuckets; ++i) {
    dts_hashtable_node *current = ht->buckets[i];
    while (current) {
      if (destroy_pair) {
        destroy_pair(ht, current->key, current->obj);
      } else {
        if (ht->keylen) {
          free(current->key);
        }
        if (current->objsize) {
          free(current->obj);
        }
      }
      free(current);
    }
  }

  free(ht->buckets);
  free(ht);
}

void *dts_hashtable_insert(
    dts_hashtable *ht, void const *key, void const *obj
) {
  return dts_hashtable_insert_extra(ht, key, obj, ht->objsize);
}
void *dts_hashtable_insert_extra(
    dts_hashtable *ht, void const *key, void const *obj, size_t objsize
) {
  size_t              hash    = ht->hash(key);
  dts_hashtable_node *newnode = calloc(1, sizeof(dts_hashtable_node));
  if (!newnode) {
    return 0;
  }

  dts_hashtable_node **bucket = ht->buckets + hash % ht->nbuckets;

  // TODO: Check if resizing is needed

  newnode->next       = *bucket;
  newnode->next_count = (*bucket) ? (*bucket)->next_count + 1 : 1;

  if (ht->keylen) {
    size_t keylen = ht->keylen(key);
    newnode->key  = calloc(1, keylen);
    memcpy(newnode->key, key, keylen);
  } else {
    newnode->key = (void *)key;
  }

  if (objsize) {
    newnode->obj = calloc(1, objsize);
    memcpy(newnode->obj, obj, objsize);
  } else {
    newnode->obj = (void *)obj;
  }

  newnode->objsize = objsize;

  *bucket = newnode;
  return newnode->obj;
}
void *dts_hashtable_search(dts_hashtable *ht, void const *key, bool *found) {
  size_t              hash    = ht->hash(key);
  dts_hashtable_node *current = ht->buckets[hash % ht->nbuckets];

  while (current) {
    if (!ht->keycmp(key, current->key)) {
      return current->obj;
    }
    if (found) {
      *found = true;
    }
    current = current->next;
  }
  if (found) {
    *found = false;
  }
  return 0;
}
void dts_hashtable_remove(dts_hashtable *ht, void const *key, void *out_obj) {
  // TODO
}

size_t dts_hashtable_std_strhash(void const *vkey) {
  // from https://stackoverflow.com/a/57960443/21296545
  char const *key = vkey;

  size_t h = 525201411107845655ull;
  while (*key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
    ++key;
  }
  return h;
}
size_t dts_hashtable_std_uptrhash(void const *key) {
  return (size_t)key;
}
