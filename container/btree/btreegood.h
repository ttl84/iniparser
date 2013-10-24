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
calloc(1, (sizeof(struct name##node) + 7) / 8 * 8)

#define BNODE_DEALLOC(nd)\
free(nd)

#define BTREE_DEFINE_STRUCT(name, type, deg)\
struct name##node{\
	unsigned size;/* number of keys (< degree)*/\
	type items[deg - 1];\
	struct name##node * nodes[deg];\
};\
struct name{\
	struct name##node * root;\
};

#define BTREE_DEFINE_INIT(name, type, deg)\
static int name##_init(struct name * bt){\
	bt->root = calloc(1, sizeof(struct name##node));\
	if(bt->root)\
		return 0;\
	else\
		return 1;\
}\
static void name##_destroy_tree(struct name##node * t){\
	if(t){\
		for(unsigned i = 0, end = t->size; i < end; i++)\
			name##_destroy_tree(t->nodes[i]);\
		BNODE_DEALLOC(t);\
	}\
}\
static void name##_destroy(struct name * bt){\
	name##_destroy_tree(bt->root);\
}

#define BTREE_DEFINE_FIND(name, type, cmp, deg)\
static int name##_find(struct name const * restrict bt, type item, type * restrict ret)\
{\
{\
	struct name##node * restrict nd = bt->root;\
	while(1){\
		int mid = 0;\
		int result = 0;\
		for(int lo = 0, hi = nd->size - 1; lo <= hi;){\
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
		nd = nd->nodes[mid + (result > 0)];\
	}\
}\
}

#define BTREE_DEFINE_INSERT(name, type, cmp, deg)\
static struct name##node * name##node##split(\
	struct name##node * restrict cur,\
	struct name##node * restrict par,\
	unsigned const pos)\
{\
	struct name##node * restrict split = BNODE_ALLOC(name);\
	memcpy(split->items, cur->items + (deg / 2),\
		(deg / 2 - 1) * sizeof(type));\
	memcpy(split->nodes, cur->nodes + (deg / 2),\
		(deg / 2) * sizeof cur);\
	memmove(par->items + pos + 1, par->items + pos,\
		(par->size - pos) * sizeof(type));\
	memmove(par->nodes + pos + 2, par->nodes + pos + 1,\
		(par->size - pos) * sizeof cur);\
	par->items[pos] = cur->items[deg / 2 - 1];\
	par->nodes[pos + 1] = split;\
	par->size++;\
	split->size = deg / 2 - 1;\
	cur->size = deg / 2 - 1;\
	return split;\
}\
static int name##_insert(struct name * restrict bt, type item){\
	struct name##node * cur = bt->root;\
	struct name##node * par = NULL;\
	if(cur->size == deg - 1){\
		par = BNODE_ALLOC(name);\
		par->nodes[0] = bt->root;\
		par->size = 0;\
		bt->root = par;\
	}\
	unsigned pos = 0;\
	while(1){\
		int mid = 0;\
		for(int lo = 0, hi = cur->size - 1; lo <= hi;){\
			mid = (lo + hi) >> 1;\
			int result = cmp(item, cur->items[mid]);\
			if(result < 0)\
				hi = mid - 1;\
			else if(result > 0)\
				lo = ++mid;\
			else\
				return 1;\
		}\
		if(cur->size == deg - 1){\
			struct name##node * split = name##node##split(cur, par, pos);\
			if(mid >= (deg / 2)){\
				mid -= deg / 2;\
				cur = split;\
			}\
		}\
		if(cur->nodes[0]){/* this is kind of like recursion*/\
			par = cur; cur = cur->nodes[mid]; pos = mid;\
		}else{\
			memmove(cur->items + mid + 1, cur->items + mid,\
				(cur->size - mid) * sizeof(type));\
			cur->items[mid] = item;\
			cur->size++;\
			return 0;\
		}\
	}\
}

#define BTREE_DEFINE_REMOVE(name, type, cmp, deg)\
static void name##node##_merge_child(\
	struct name##node * cur,\
	unsigned const pos)\
{\
	struct name##node * const l = cur->nodes[pos];\
	struct name##node * const r = cur->nodes[pos + 1];\
	memcpy(l->items + deg / 2, r->items,\
		(deg / 2 - 1) * sizeof(type));\
	memcpy(l->nodes + deg / 2, r->nodes,\
		(deg / 2) * sizeof cur);\
	l->items[deg / 2 - 1] = cur->items[pos];\
	memmove(cur->items + pos, cur->items + pos + 1,\
		(cur->size - pos - 1) * sizeof(type));\
	memmove(cur->nodes + pos + 1, cur->nodes + pos + 2,\
		(cur->size - pos - 1) * sizeof cur);\
	BNODE_DEALLOC(r);\
	l->size = deg - 1;\
	cur->size--;\
}\
static int name##_remove(\
	struct name * const bt,\
	type item)\
{\
	struct name##node * cur = bt->root;\
	unsigned pos = 0;\
	struct name##node * targ_cur = NULL;\
	unsigned targ_pos = 0;\
	while(1){\
		if(!targ_cur){\
			int mid = 0;\
			for(int lo = 0, hi = cur->size - 1; lo <= hi;){\
				mid = (lo + hi) >> 1;\
				int result = cmp(item, cur->items[mid]);\
				if(result < 0)\
					hi = mid - 1;\
				else if(result > 0)\
					lo = ++mid;\
				else{\
					targ_pos = mid;\
					targ_cur = cur;\
					break;\
				}\
			}\
			pos = mid;\
		}else{\
			pos = cur->size;\
		}\
		if(cur->nodes[0] == 0)\
			break;\
		struct name##node * next = cur->nodes[pos];\
		if(next->size == deg / 2 - 1){\
			if(pos != cur->size){\
				struct name##node * right = cur->nodes[pos + 1];\
				unsigned const rsiz = right->size;\
				if(rsiz != deg / 2 - 1){\
					next->items[deg / 2 - 1] =\
						cur->items[pos];\
					next->nodes[deg / 2] =\
						right->nodes[0];\
					cur->items[pos] =\
						right->items[0];\
					memmove(right->items, right->items + 1,\
						(rsiz - 1) * sizeof(type));\
					memmove(right->nodes, right->nodes + 1,\
						rsiz * sizeof cur);\
					right->size--;\
					next->size++;\
				}else{\
					name##node##_merge_child(cur, pos);\
				}\
				if(cur == targ_cur)\
					targ_cur = 0;\
			}else{\
				struct name##node * left = cur->nodes[pos - 1];\
				unsigned const lsiz = left->size;\
				if(lsiz != deg / 2 - 1){\
					memmove(next->items + 1, next->items,\
						(deg / 2 - 1) * sizeof(type));\
					memmove(next->nodes + 1, next->nodes,\
						(deg / 2) * sizeof cur);\
					next->items[0] = cur->items[pos - 1];\
					next->nodes[0] =\
						left->nodes[lsiz];\
					cur->items[pos - 1] =\
						left->items[lsiz - 1];\
					left->size--;\
					next->size++;\
				}else{\
					name##node##_merge_child(cur, pos - 1);\
					next = left;\
				}\
			}\
		}\
		cur = next;\
	}\
	if(targ_cur){\
	/* swap with predecessor*/\
		if(cur == targ_cur){\
			memmove(cur->items + pos, cur->items + pos + 1,\
				(cur->size - 1 - pos) * sizeof(type));\
		}else{\
			targ_cur->items[targ_pos] = cur->items[pos - 1];\
		}\
		cur->size--;\
		struct name##node * old_root = bt->root;\
		if(old_root->size == 0){\
			struct name##node * new_root = old_root->nodes[0];\
			if(new_root){\
				bt->root = new_root;\
				BNODE_DEALLOC(old_root);\
			}\
		}\
		return 0;\
	}else\
		return 1;\
}
#endif
