#ifndef BTREE_H
#define BTREE_H

/* make btree with many parameters*/
#define BTREE_MAKE_WITH(name, type, cmp, deg)\
BTREE_DEFINE_STRUCT(name, type, deg)\
BTREE_DEFINE_INIT(name, type, deg)\
BTREE_DEFINE_FIND(name, type, cmp, deg)\
BTREE_DEFINE_INSERT(name, type, cmp, deg)\
BTREE_DEFINE_REMOVE(name, type, cmp, deg)

/* make btree with some default parameters*/
#define BTREE_MAKE(name, type, cmp, deg)\
BTREE_MAKE_WITH(name, type, cmp, BTREE_DEG(deg))

#define BTREE_DEG(deg)\
(((deg) + 4) & ~1)


#define BTREE_DEFINE_STRUCT(name, type, deg)\
struct bnode##name{\
	type items[deg - 1];\
	unsigned nodes[deg];\
	unsigned size;/* number of keys (< degree)*/\
};\
struct name{\
	struct bnode##name * mem;\
	unsigned root;\
	unsigned last;/* position of next free space*/\
	unsigned size;/* size of mem*/\
	int height;\
};

#define BNODE_INIT(nd, deg)\
do{\
	for(unsigned i = deg; i--;)\
		(nd).nodes[i] = 0;\
	(nd).size = 0;\
}while(0)

#define BTREE_DEFINE_INIT(name, type, deg)\
static int name##_init(struct name * bt){\
	bt->root = 1;\
	bt->last = 2;\
	bt->size = 3;\
	bt->height = 1;\
	bt->mem = calloc(bt->size,  sizeof(struct bnode##name));\
	if(bt->mem){\
		BNODE_INIT(bt->mem[bt->root], (deg));\
		return 0;\
	}\
	return 1;\
}

#define BTREE_DEFINE_FIND(name, type, cmp, deg)\
static int name##_find(struct name const * const restrict bt, type item, type * ret){\
	struct bnode##name const * const restrict m = bt->mem;\
	unsigned root = bt->root;\
	while(root){\
		int mid = 0;\
		int lo = 0;\
		int hi = m[root].size - 1;\
		int result = 0;\
		while(lo <= hi){\
			mid = (lo + hi) >> 1;\
			result = cmp(item, m[root].items[mid]);\
			if(result < 0)\
				hi = mid - 1;\
			else if(result > 0)\
				lo = mid + 1;\
			else{\
				*ret = m[root].items[mid];\
				return 0;\
			}\
		}\
		if(m[root].nodes[0] == 0)\
			break;\
		root = m[root].nodes[mid + (result > 0)];\
	}\
	return 1;\
}

/* btree equivalent of malloc; get a new block of memory for a node*/
#define BNODE_ALLOC(bt)\
(bt->last++)

/* makes sure memory is big enough*/
#define BTREE_RESERVE(bt,n, name)\
do{\
	unsigned reqed = (n);\
	if(reqed > bt->size){\
		reqed *= 2;\
		struct bnode##name * m = realloc(bt->mem, reqed * sizeof *m);\
		if(m != NULL){\
			bt->mem = m;\
			bt->size = reqed;\
		}\
	}\
}while(0)
#define BTREE_DEFINE_INSERT(name, type, cmp, deg)\
static int name##_insert(struct name * restrict bt, type item){\
	unsigned self = bt->root;\
	unsigned parent = 0;\
	unsigned pos = 0;\
	int quit = 0;\
	BTREE_RESERVE(bt, bt->last + bt->height + 2, name);\
	struct bnode##name * const restrict  m = bt->mem;\
	if(m[self].size == deg - 1){\
		parent = BNODE_ALLOC(bt);\
		m[parent].nodes[0] = bt->root;\
		m[parent].size = 0;\
		bt->root = parent;\
		bt->height++;\
	}\
	while(1){\
		int mid = 0, lo = 0, hi = m[self].size - 1;\
		int result = 0;\
		while(lo <= hi){\
			mid = (lo + hi) >> 1;\
			result = cmp(item, m[self].items[mid]);\
			if(result < 0)\
				hi = mid - 1;\
			else if(result > 0)\
				lo = ++mid;\
			else{\
				quit = 1;\
				break;\
			}\
		}\
		if(m[self].size == deg - 1){\
			unsigned split = BNODE_ALLOC(bt);\
			memcpy(m[split].items, m[self].items + (deg / 2),\
				(deg / 2 - 1) * sizeof(type));\
			memcpy(m[split].nodes, m[self].nodes + (deg / 2),\
				(deg / 2) * sizeof self);\
			memmove(m[parent].items + pos + 1, m[parent].items + pos,\
				(m[parent].size - pos) * sizeof(type));\
			memmove(m[parent].nodes + pos + 2, m[parent].nodes + pos + 1,\
				(m[parent].size - pos) * sizeof self);\
			m[parent].items[pos] = m[self].items[deg / 2 - 1];\
			m[parent].nodes[pos + 1] = split;\
			m[parent].size++;\
			m[split].size = deg / 2 - 1;\
			m[self].size = deg / 2 - 1;\
			if(mid >= (deg / 2)){\
				mid -= deg / 2;\
				self = split;\
			}\
		}\
		pos = mid;\
		if(m[self].nodes[0] == 0)\
			break;\
		parent = self;\
		self = m[self].nodes[pos];\
	}\
	if(quit)\
		return 1;\
	memmove(m[self].items + pos + 1, m[self].items + pos,\
		(m[self].size - pos) * sizeof(type));\
	m[self].items[pos] = item;\
	m[self].size++;\
	return 0;\
}
#define BTREE_DEFINE_REMOVE(name, type, cmp, deg)\
static inline void bnode##name_dealloc(struct bnode##name * nd){\
	nd->size = (deg);\
	nd->nodes[0] = 0;\
}\
static void bnode##name_merge_child(struct bnode##name * const restrict m,\
	unsigned const self, unsigned const pos){\
	unsigned const l = m[self].nodes[pos];\
	unsigned const r = m[self].nodes[pos + 1];\
	memcpy(m[l].items + deg / 2,\
		m[r].items,\
		(deg / 2 - 1) * sizeof(type));\
	memcpy(m[l].nodes + deg / 2,\
		m[r].nodes,\
		(deg / 2) * sizeof self);\
	m[l].items[deg / 2 - 1] = m[self].items[pos];\
	memmove(m[self].items + pos,\
		m[self].items + pos + 1,\
		(m[self].size - pos - 1) * sizeof(type));\
	memmove(m[self].nodes + pos + 1,\
		m[self].nodes + pos + 2,\
		(m[self].size - pos - 1) * sizeof self);\
	bnode##name_dealloc(m + r);\
	m[l].size = deg - 1;\
	m[self].size--;\
}\
static int name##_remove(struct name * const bt, type item){\
	struct bnode##name * const m = bt->mem;\
	unsigned self = bt->root;\
	unsigned pos = 0;\
	struct targ{unsigned self; unsigned pos;} target = {0, 0};\
	while(1){\
		if(target.self)\
			pos = m[self].size;\
		else{\
			int mid= 0, lo = 0, hi = m[self].size - 1;\
			while(lo <= hi){\
				mid = (lo + hi) >> 1;\
				int result = cmp(item, m[self].items[mid]);\
				if(result < 0)\
					hi = mid - 1;\
				else if(result > 0)\
					lo = ++mid;\
				else{\
					target.pos = mid;\
					target.self = self;\
					break;\
				}\
			}\
			pos = mid;\
		}\
		if(m[self].nodes[0] == 0)\
			break;\
		unsigned next = m[self].nodes[pos];\
		if(m[next].size == deg / 2 - 1){\
			if(pos == m[self].size){\
				unsigned const left = m[self].nodes[pos - 1];\
				if(m[left].size == deg / 2 - 1){\
					bnode##name_merge_child(m, self, pos - 1);\
					next = left;\
				}else{\
					memmove(m[next].items + 1,\
						m[next].items,\
						(deg / 2 - 1) * sizeof(type));\
					memmove(m[next].nodes + 1,\
						m[next].nodes,\
						(deg / 2) * sizeof self);\
					m[next].items[0] = m[self].items[pos - 1];\
					unsigned const lsize = m[left].size;\
					m[next].nodes[0] =\
						m[left].nodes[lsize];\
					m[self].items[pos - 1] =\
						m[left].items[lsize - 1];\
					m[left].size--;\
					m[next].size++;\
				}\
			}else{\
				unsigned const right = m[self].nodes[pos + 1];\
				if(m[right].size == deg / 2 - 1){\
					bnode##name_merge_child(m, self, pos);\
				}else{\
					m[next].items[deg / 2 - 1] =\
						m[self].items[pos];\
					m[next].nodes[deg / 2] =\
						m[right].nodes[0];\
					m[self].items[pos] =\
						m[right].items[0];\
					memmove(m[right].items,\
						m[right].items + 1,\
						(m[right].size - 1) * sizeof(type));\
					memmove(m[right].nodes,\
						m[right].nodes + 1,\
						m[right].size * sizeof self);\
					m[right].size--;\
					m[next].size++;\
				}\
				if(self == target.self)\
					target.self = 0;\
			}\
		}\
		self = next;\
	}\
	if(!target.self)\
		return 1;\
	/* swap with predecessor*/\
	if(self != target.self)\
		m[target.self].items[target.pos] = m[self].items[pos - 1];\
	else\
		memmove(m[self].items + pos,\
			m[self].items + pos + 1,\
			(m[self].size - 1 - pos) * sizeof(type));\
	m[self].size--;\
	unsigned old_root = bt->root;\
	if(m[old_root].size == 0){\
		unsigned new_root = m[old_root].nodes[0];\
		if(new_root){\
			bt->root = new_root;\
			bnode##name_dealloc(m + old_root);\
			bt->height--;\
		}\
	}\
	return 0;\
}
#endif
