#ifndef SET_H
#define SET_H
#include "list.h"
typedef int (*set_cmp)(void const *, void const *);
typedef void * (*set_cpy)(void const *);
typedef void (*set_dtor)(void *);
struct set;
struct set * set_new(set_cmp cmp, set_cpy, set_dtor dtor);
void set_del(struct set * );
int set_insert(struct set *, void const *);
int set_remove(struct set *, void const *);
#endif