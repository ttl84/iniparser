#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "hashset.h"
#define HASHTABLE_MAKE(name, type1, type2, CMP, HASH)\
struct name##_pair{\
	type1 key;\
	type2 val;\
};\
static inline int name##compare(struct name##_pair a, struct name##_pair b)\
{\
	return CMP(a.key, b.key);\
}\
static inline size_t name##hash(struct name##_pair a)\
{\
	return HASH(a.key);\
}\
HASHSET_MAKE(name, struct name##_pair, name##compare, name##hash)\
static int name##_haskey(struct name const * ht, type1 key)\
{\
	struct name##_pair pair = {.key = key};\
	return name##_has(ht, pair);\
}\
static type2 name##_getval(struct name const * ht, type1 key)\
{\
	struct name##_pair pair = {.key = key};\
	pair = name##_get(ht, pair);\
	return pair.val;\
}\
static int name##_setval(struct name * ht, type1 key, type2 val)\
{\
	struct name##_pair pair = {.key = key, .val = val};\
	return name##_insert(ht, pair);\
}\
static int name##_delkey(struct name * ht, type1 key)\
{\
	struct name##_pair pair = {.key = key};\
	return name##_remove(ht, pair);\
}
#endif
