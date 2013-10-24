#ifndef BTREEV_H
#define BTREEV_H
typedef int (*bt_cmp)(void const *, void const *);
struct btree;
struct btree * btree_new(unsigned ele_size, unsigned deg, bt_cmp cmp);
void btree_del(struct btree *);
void const * btree_get(struct btree const * bt, void const * item);
int btree_insert(struct btree * bt, void const * item);
int btree_remove(struct btree * bt, void const * item);
#endif
