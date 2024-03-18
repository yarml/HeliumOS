// Rational: For some unknown reason using BTreeMap occasionally crashes.

use crate::println;
use alloc::vec::Vec;
use core::{
  hash::{Hash, Hasher},
  mem,
};

use crate::{feat::hash::DefaultHasher, mem::early_heap::EarlyAllocator};

pub struct HashMap<K, V>
where
  K: Hash + Eq + Clone,
  V: Clone,
{
  buckets: Vec<Vec<Node<K, V>>>, // >>>>>>>>>>>>>>>>>>
}

struct Node<K, V>
where
  K: Eq,
{
  key: K,
  value: V,
}

impl<K, V> HashMap<K, V>
where
  K: Hash + Eq + Clone,
  V: Clone,
{
  const MAX_COLLISIONS: usize = 8;
  const INIT_BUCKETS: usize = 32;
  pub fn new() -> Self {
    let mut buckets = Vec::new();
    for _ in 0..Self::INIT_BUCKETS {
      buckets.push(Vec::new());
    }

    let bucket = &buckets[19];
    println!("{:?}", bucket as *const Vec<Node<K, V>>);

    Self { buckets }
  }

  pub fn insert(&mut self, key: K, value: V) -> Option<V> {
    let hash = Self::hashof(&key);
    let index = hash as usize % self.buckets.len();

    let bucket = &mut self.buckets[index];

    for node in bucket.iter_mut() {
      if node.key == key {
        let old_value = node.value.clone();
        node.value = value;
        return Some(old_value);
      }
    }

    // If we're here, then this is a new key

    // TODO: Check if we surpassed max collisions
    bucket.push(Node { key, value });
    None
  }

  fn hashof(key: &K) -> u64 {
    let mut hasher = DefaultHasher::new();
    key.hash(&mut hasher);
    hasher.finish()
  }
}
