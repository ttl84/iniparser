#ifndef BTREE_H
#define BTREE_H
/* make btree with some default parameters*/
#define BTREE_MAKE(name, type, cmp, deg)\
BTREE_MAKE_WITH(name, type, cmp, BTREE_DEG(deg))

/* make btree with many parameters*/
#define BTREE_MAKE_WITH(name, type, cmp, deg)\
BTREE_DEFINE_STRUCT(name, type, deg)\
BTREE_DEFINE_INIT(name, type, deg)\
BTREE_DEFINE_FIND(name, type, cmp, deg)\
BTREE_DEFINE_INSERT(name, type, cmp, deg)\
BTREE_DEFINE_REMOVE(name, type, cmp, deg)

/* make sure degree is even and is at least 4*/
#define BTREE_DEG(deg)\
((deg) >= 4 ? (deg) & ~1 : 4)

#define BNODE_ALLOC(name)\
calloc(1, sizeof(struct bnode##name))

#define BNODE_DEALLOC(nd)\
free(nd)

#define BTREE_DEFINE_STRUCT(name, type, deg)\
struct bnode##name{\
	type items[deg - 1];\
	struct bnode##name * nodes[deg];\
	unsigned size;/* number of keys (< degree)*/\
};\
struct name{\
	struct bnode##name * root;\
	int height;\
};

#define BTREE_DEFINE_INIT(name, type, deg)\
int name##_init(struct name * bt){\
	bt->height = 1;\
	bt->root = calloc(1, sizeof(struct bnode##name));\
	if(bt->root)\
		return 0;\
	else\
		return 1;\
}\
void name##_destroy_tree(struct bnode##name * t){\
	if(!t)\
		return;\
	for(unsigned i = 0, end = t->size; i < end; i++)\
		name##_destroy_tree(t->nodes[i]);\
	BNODE_DEALLOC(t);\
}\
void name##_destroy(struct name * bt){\
	name##_destroy_tree(bt->root);\
}

#define BTREE_DEFINE_FIND(name, type, cmp, deg)\
static int name##_find_rec(struct bnode##name const * nd, type item, type * const ret){\
	int mid = 0;\
	int lo = 0;\
	int hi = nd->size - 1;\
	int result = 0;\
	while(lo <= hi){\
		mid = (lo + hi) >> 1;\
		result = cmp(item, nd->items[mid]);\
		if(result < 0)\
			hi = mid - 1;\
		else if(result > 0)\
			lo = mid + 1;\
		else{\
			*ret = nd->items[mid];\
			return 0;\
		}\
	}\
	if(nd->nodes[0] == 0)\
		return 1;\
	return name##_find_rec(nd->nodes[mid + (result > 0)], item, ret);\
}\
static int name##_find(struct name const * bt, type item, type * const ret){\
	return name##_find_rec(bt->root, item, ret);\
}

#define BTREE_DEFINE_INSERT(name, type, cmp, deg)\
static struct bnode##name * bnode##name##_split(\
	struct bnode##name * const self,\
	struct bnode##name * const parent,\
	unsigned const pos)\
{\
	struct bnode##name * const split = BNODE_ALLOC(name);\
	memcpy(split->items, self->items + (deg / 2),\
		(deg / 2 - 1) * sizeof(type));\
	memcpy(split->nodes, self->nodes + (deg / 2),\
		(deg / 2) * sizeof self);\
	memmove(parent->items + pos + 1, parent->items + pos,\
		(parent->size - pos) * sizeof(type));\
	memmove(parent->nodes + pos + 2, parent->nodes + pos + 1,\
		(parent->size - pos) * sizeof self);\
	parent->items[pos] = self->items[deg / 2 - 1];\
	parent->nodes[pos + 1] = split;\
	parent->size++;\
	split->size = deg / 2 - 1;\
	self->size = deg / 2 - 1;\
	return split;\
}\
static int name##_insert_rec(\
	struct bnode##name * self,\
	struct bnode##name * parent,\
	unsigned const pos, type item)\
{\
	int mid = 0;\
	int found = 0;\
	for(int lo = 0, hi = self->size - 1; lo <= hi;){\
		mid = (lo + hi) >> 1;\
		int result = cmp(item, self->items[mid]);\
		if(result < 0)\
			hi = mid - 1;\
		else if(result > 0)\
			lo = ++mid;\
		else{\
			found = 1;\
			break;\
		}\
	}\
	if(self->size == deg - 1){\
		struct bnode##name * split = bnode##name##_split(self, parent, pos);\
		if(mid >= (deg / 2)){\
			mid -= deg / 2;\
			self = split;\
		}\
	}\
	if(found){\
		return 1;\
	}else if(self->nodes[0] == 0){\
		memmove(self->items + mid + 1, self->items + mid,\
			(self->size - mid) * sizeof(type));\
		self->items[mid] = item;\
		self->size++;\
		return 0;\
	}else{\
		return name##_insert_rec(self->nodes[mid], self, mid, item);\
	}\
}\
static int name##_insert(struct name * bt, type item){\
	struct bnode##name * self = bt->root;\
	struct bnode##name * parent = NULL;\
	if(self->size == deg - 1){\
		parent = BNODE_ALLOC(name);\
		parent->nodes[0] = bt->root;\
		parent->size = 0;\
		bt->root = parent;\
		bt->height++;\
	}\
	return name##_insert_rec(self, parent, 0, item);\
}

#define BTREE_DEFINE_REMOVE(name, type, cmp, deg)\
static void bnode##name##_merge_child(\
	struct bnode##name * self,\
	unsigned const pos)\
{\
	struct bnode##name * const l = self->nodes[pos];\
	struct bnode##name * const r = self->nodes[pos + 1];\
	memcpy(l->items + deg / 2, r->items,\
		(deg / 2 - 1) * sizeof(type));\
	memcpy(l->nodes + deg / 2, r->nodes,\
		(deg / 2) * sizeof self);\
	l->items[deg / 2 - 1] = self->items[pos];\
	memmove(self->items + pos, self->items + pos + 1,\
		(self->size - pos - 1) * sizeof(type));\
	memmove(self->nodes + pos + 1, self->nodes + pos + 2,\
		(self->size - pos - 1) * sizeof self);\
	BNODE_DEALLOC(r);\
	l->size = deg - 1;\
	self->size--;\
}\
static int name##_remove(\
	struct name * const bt,\
	type item)\
{\
	struct bnode##name * self = bt->root;\
	unsigned pos = 0;\
	struct bnode##name * targ_self = NULL;\
	unsigned targ_pos = 0;\
	while(1){\
		if(targ_self)\
			pos = self->size;\
		else{\
			int mid = 0, lo = 0, hi = self->size - 1;\
			while(lo <= hi){\
				mid = (lo + hi) >> 1;\
				int result = cmp(item, self->items[mid]);\
				if(result < 0)\
					hi = mid - 1;\
				else if(result > 0)\
					lo = ++mid;\
				else{\
					targ_pos = mid;\
					targ_self = self;\
					break;\
				}\
			}\
			pos = mid;\
		}\
		if(self->nodes[0] == 0)\
			break;\
		struct bnode##name * next = self->nodes[pos];\
		if(next->size == deg / 2 - 1){\
			if(pos == self->size){\
				struct bnode##name * left = self->nodes[pos - 1];\
				unsigned const lsiz = left->size;\
				if(lsiz == deg / 2 - 1){\
					bnode##name##_merge_child(self, pos - 1);\
					next = left;\
				}else{\
					memmove(next->items + 1, next->items,\
						(deg / 2 - 1) * sizeof(type));\
					memmove(next->nodes + 1, next->nodes,\
						(deg / 2) * sizeof self);\
					next->items[0] = self->items[pos - 1];\
					next->nodes[0] =\
						left->nodes[lsiz];\
					self->items[pos - 1] =\
						left->items[lsiz - 1];\
					left->size--;\
					next->size++;\
				}\
			}else{\
				struct bnode##name * right = self->nodes[pos + 1];\
				unsigned const rsiz = right->size;\
				if(rsiz == deg / 2 - 1){\
					bnode##name##_merge_child(self, pos);\
				}else{\
					next->items[deg / 2 - 1] =\
						self->items[pos];\
					next->nodes[deg / 2] =\
						right->nodes[0];\
					self->items[pos] =\
						right->items[0];\
					memmove(right->items, right->items + 1,\
						(rsiz - 1) * sizeof(type));\
					memmove(right->nodes, right->nodes + 1,\
						rsiz * sizeof self);\
					right->size--;\
					next->size++;\
				}\
				if(self == targ_self)\
					targ_self = 0;\
			}\
		}\
		self = next;\
	}\
	if(!targ_self)\
		return 1;\
	/* swap with predecessor*/\
	if(self != targ_self)\
		targ_self->items[targ_pos] = self->items[pos - 1];\
	else\
		memmove(self->items + pos, self->items + pos + 1,\
			(self->size - 1 - pos) * sizeof(type));\
	self->size--;\
	struct bnode##name * old_root = bt->root;\
	if(old_root->size == 0){\
		struct bnode##name * new_root = old_root->nodes[0];\
		if(new_root){\
			bt->root = new_root;\
			BNODE_DEALLOC(old_root);\
			bt->height--;\
		}\
	}\
	return 0;\
}
#endif
