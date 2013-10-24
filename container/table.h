#ifndef TABLE_H_H_H
#define TABLE_H_H_H
#include <stddef.h>
#include <stdbool.h>
typedef int (* ht_cmp)(void const * key1, void const * key2);

typedef size_t (* ht_hash)(void const * key);

struct ht;

struct ht * ht_new(size_t n_buckets, ht_cmp cmp_ptr, ht_hash hash_ptr);
void ht_del(struct ht * table);

size_t ht_size(struct ht * table);
size_t ht_nitem(struct ht * table);

bool ht_haskey(struct ht *, void const * key);
void * ht_get(struct ht * table, void const * key);

int ht_insert(struct ht * table, void * key, void * val);
int ht_remove(struct ht * table, void const * key, void ** retkey, void ** retval);

int ht_grow(struct ht *);

struct ht_iter;
struct ht_iter * ht_iter_new(void);
void ht_iter_del(struct ht_iter *);
struct ht_iter * ht_iter_init(struct ht_iter *, struct ht *);
void * ht_key(struct ht_iter *);
void * ht_val(struct ht_iter *);
bool ht_next(struct ht_iter *);
#endif
