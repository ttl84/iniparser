#ifndef HASHSET_H
#define HASHSET_H
#define HASHSET_MOD(i, n)\
((i) & ((n) - 1))

#define HASHSET_MAKE(name, type, cmp, hash)\
struct name##link{\
	struct name##link * next;\
	type key;\
};\
struct name{\
	struct name##link ** buks;\
	size_t size;\
	size_t nitem;\
};\
int name##_init(struct name * hs, size_t size)\
{\
	{\
		size_t p;\
		for(p = 1; p < size; p *= 2){}\
		size = p;\
	}\
	struct name##link ** buks = calloc(size, sizeof *buks);\
	if(buks == NULL)\
		return 1;\
	hs->buks = buks;\
	hs->size = size;\
	hs->nitem = 0;\
	return 0;\
}\
void name##_destroy(struct name * hs)\
{\
	struct name##link ** buks = hs->buks;\
	for(size_t i = hs->size; i--;)\
	{\
		while(buks[i])\
		{\
			struct name##link * deleted = buks[i];\
			buks[i] = deleted->next;\
			free(deleted);\
		}\
	}\
	free(buks);\
	hs->buks = NULL;\
	hs->nitem = 0;\
}\
int name##_has(struct name const * hs, type key)\
{\
	size_t i = HASHSET_MOD(hash(key), hs->size);\
	struct name##link * iter = hs->buks[i];\
	while(iter)\
	{\
		if(0 == cmp(key, iter->key))\
			return 1;\
		iter = iter->next;\
	}\
	return 0;\
}\
type name##_get(struct name const * hs, type key)\
{\
	size_t i = HASHSET_MOD(hash(key), hs->size);\
	struct name##link * iter = hs->buks[i];\
	while(iter)\
	{\
		if(0 == cmp(key, iter->key))\
			return iter->key;\
		iter = iter->next;\
	}\
	return key;\
}\
int name##_insert(struct name * hs, type key)\
{\
	struct name##link * key_node = calloc(1, sizeof *key_node);\
	size_t i = HASHSET_MOD(hash(key), hs->size);\
	key_node->key = key;\
	key_node->next = hs->buks[i];\
	hs->buks[i] = key_node;\
	hs->nitem++;\
	return 0;\
}\
int name##_remove(struct name * hs, type key)\
{\
	size_t i = HASHSET_MOD(hash(key), hs->size);\
	struct name##link * iter = hs->buks[i];\
	struct name##link head = {.next = iter};\
	struct name##link * prev = &head;\
	while(iter)\
	{\
		if(0 != cmp(key, iter->key))\
		{\
			prev = iter;\
			iter = iter->next;\
		}\
		else\
		{\
			prev->next = iter->next;\
			hs->buks[i] = head.next;\
			free(iter);\
			hs->nitem--;\
			return 0;\
		}\
	}\
	return -1;\
}\
int name##_grow(struct name * hs)\
{\
	size_t old_size = hs->size;\
	size_t new_size = old_size * 2;\
	if(new_size <= old_size)\
		return 1;\
	struct name##link ** old_buks = hs->buks;\
	struct name##link ** new_buks = calloc(new_size, sizeof *new_buks);\
	/* failed to allocate bigger array*/\
	if(new_buks == NULL)\
		return 2;\
	for(size_t i = 0; i < old_size; i++)\
	{\
		/* find a non empty bucket*/\
		while(old_buks[i])\
		{\
			/* pop the bucket*/\
			struct name##link * top = old_buks[i];\
			old_buks[i] = top->next;\
			/* rehash the key*/\
			size_t i2 = HASHSET_MOD(hash(top->key), new_size);\
			top->next = new_buks[i2];\
			new_buks[i2] = top;\
		}\
	}\
	free(old_buks);\
	hs->size = new_size;\
	hs->buks = new_buks;\
	return 0;\
}\
struct name##_iterator{\
	type item;\
	struct name##link * current;\
	struct name##link * next;\
	struct name * hs;\
	size_t i;\
};\
struct name##_iterator name##_iter(struct name * hs)\
{\
	struct name##_iterator iter = {\
		.hs = hs,\
		.current = NULL,\
		.next = hs->buks[0],\
		.i = 0\
	};\
	return iter;\
}\
int name##_next(struct name##_iterator * iter)\
{\
	struct name const * hs = iter->hs;\
	if(iter->i < hs->size)\
	{\
		while(iter->next == NULL)\
		{\
			iter->i++;\
			if(iter->i < hs->size)\
				iter->next = hs->buks[iter->i];\
			else\
				return 0;\
		}\
		iter->current = iter->next;\
		iter->next = iter->next->next;\
		iter->item = iter->current->key;\
		return 1;\
	}\
	else\
		return 0;\
}
#endif
