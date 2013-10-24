#ifndef HASHSETV_H
#define HASHSETV_H
typedef unsigned (*hs_hash)(void const *);
typedef int (*hs_cmp)(void const *, void const *);
struct hashset;
struct hashset * hashset_new(unsigned ele_size, hs_cmp cmp, hs_hash);
void hashset_del(struct hashset *);
void * hashset_get(struct hashset *, void const *);
int hashset_insert(struct hashset *, void const *);
int hashset_insert_s(struct hashset *, void const *, unsigned size);
int hashset_remove(struct hashset *, void const *);
#endif
