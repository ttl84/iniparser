#ifndef BTREE_H
#define BTREE_H

/* make btree with many parameters*/
#define BTREE_MAKE_WITH(name, type, cmp, deg, malloc, realloc, free, growth)\
BTREE_DEFINE_STRUCT(name, type, BTREE_DEG(deg))\
BTREE_DEFINE_INIT(name, type, BTREE_DEG(deg), malloc)\
BTREE_DEFINE_FIND(name, type, cmp, BTREE_DEG(deg))\
BTREE_DEFINE_INSERT(name, type, cmp, BTREE_DEG(deg), realloc)\
BTREE_DEFINE_REMOVE(name, type, cmp, BTREE_DEG(deg))

/* make btree with some default parameters*/
#define BTREE_MAKE(name, type, cmp, deg)\
BTREE_MAKE_WITH(name, type, cmp, (deg), malloc, realloc, free, 2)

#define BTREE_DEG(deg)\
((deg) | 3)

/* btree equivalent of malloc; get a new block of memory for a node*/
#define BNODE_ALLOC(bt)\
(bt->nelem++)

#define BTREE_RESERVE(bt, n, name, realloc)\
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

#define BTREE_DEFINE_STRUCT(name, type, degree)\
struct bnode##name{\
	type items[(degree) - 1];\
	unsigned nodes[(degree)];\
	unsigned size;/* number of keys (< degree)*/\
};\
struct name{\
	struct bnode##name * mem;\
	unsigned root;\
	unsigned nelem;/* position of next free space*/\
	unsigned size;/* size of mem*/\
	unsigned probe;/* checks for free spaces in fragmented mem*/\
};

#define BNODE_INIT(nd, deg)\
do{\
	for(unsigned i = (deg); i--;)\
		(nd).nodes[i] = 0;\
	(nd).size = 0;\
}while(0)

#define BTREE_DEFINE_INIT(name, type, deg, malloc)\
int name##_init(struct name * bt){\
	printf("degree: %d\n", (deg));\
	bt->root = 2;\
	bt->nelem = 3;\
	bt->size = 3;\
	bt->mem = malloc(bt->size * sizeof(struct bnode##name));\
	if(bt->mem){\
		BNODE_INIT(bt->mem[bt->root], (deg));\
		return 0;\
	}\
	return 1;\
}

#define BTREE_DEFINE_FIND(name, type, cmp, deg)\
static int name##_find(struct name const * const restrict bt, type item, type * ret){\
	struct bnode##name const * const restrict m = bt->mem;\
	register unsigned root = bt->root;\
	while(root){\
		register int mid = 0;\
		register int lo = 0;\
		register int hi = m[root].size - 1;\
		register int result = 0;\
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
	
#define BTREE_DEFINE_INSERT(name, type, cmp, deg, realloc)\
static int name##_insert(struct name * bt, type item){\
	struct bnode##name * m = bt->mem;\
	struct frame{\
		unsigned self;\
		unsigned pos;\
	} stack[64];\
	register unsigned sp = 0;\
	register unsigned self = bt->root;\
	register unsigned pos;\
	while(1){\
		/* search for the hole to insert into*/\
		register int mid = 0, lo = 0, hi = m[self].size - 1;\
		register int result = 0;\
		while(lo <= hi){\
			mid = (lo + hi) >> 1;\
			result = cmp(item, m[self].items[mid]);\
			if(result < 0)\
				hi = mid - 1;\
			else if(result > 0)\
				lo = mid + 1;\
			else\
				return 1;\
		}\
		pos = mid + (result > 0);\
		/* push*/\
		sp++;\
		stack[sp].self = self;\
		stack[sp].pos = pos;\
		if(m[self].nodes[0] == 0)\
			break;\
		else\
			self = m[self].nodes[pos];\
	}\
	/* make sure there are enough space for all the splits to come*/\
	BTREE_RESERVE(bt, (bt->nelem + sp + 1), name, realloc);\
	m = bt->mem;\
	type extra = item;\
	unsigned sub_split = 0;\
	while(sp){\
		/* pop*/\
		pos = stack[sp].pos;\
		self = stack[sp].self;\
		sp--;\
		unsigned node_size = m[self].size;\
		if(node_size + 1 < (deg)){\
		/* no need to split, so first make room*/\
			memmove(m[self].items + pos + 1,\
				m[self].items + pos,\
				(node_size - pos) * sizeof(type));\
			memmove(m[self].nodes + pos + 2,\
				m[self].nodes + pos + 1,\
				(node_size - pos) * sizeof self);\
			/*for(unsigned i = node_size; i > pos; i--){\
				m[self].items[i] = m[self].items[i - 1];\
				m[self].nodes[i + 1] = m[self].nodes[i];\
			}*/\
			/* then put the extra and split into the node*/\
			m[self].items[pos] = extra;\
			m[self].nodes[pos + 1] = sub_split;\
			m[self].size++;\
			return 0;\
		}\
		/* otherwise need to split, split is always on right side*/\
		unsigned my_split = BNODE_ALLOC(bt);\
		/* pick one of three ways to insert, depending on position*/\
		if(pos < ((deg) / 2)){\
			/* insert before middle*/\
			/* first copy right half to split*/\
			memcpy(m[my_split].items,\
				m[self].items + ((deg) / 2),\
				(deg) / 2 * sizeof(type));\
			memcpy(m[my_split].nodes,\
				m[self].nodes + ((deg) / 2),\
				(deg) / 2 * sizeof self);\
			/*for(unsigned i = 0; i < ((deg) / 2); i++){\
				m[my_split].items[i] = m[self].items[i + (deg) / 2];\
				m[my_split].nodes[i] = m[self].nodes[i + (deg) / 2];\
			}*/\
			m[my_split].nodes[(deg) / 2] = m[self].nodes[(deg) / 2 * 2];\
			/* then make room for extra*/\
			memmove(m[self].items + pos + 1,\
				m[self].items + pos,\
				((deg) / 2 - pos) * sizeof(type));\
			memmove(m[self].nodes + pos + 1,\
				m[self].nodes + pos,\
				((deg) / 2 - pos) * sizeof self);\
			/*for(unsigned i = (deg) / 2; i > pos; i--){\
				m[self].items[i] = m[self].items[i - 1];\
				m[self].nodes[i] = m[self].nodes[i - 1];\
			}*/\
			/* then put*/\
			m[self].items[pos] = extra;\
			m[self].nodes[pos + 1] = sub_split;\
			extra = m[self].items[(deg) / 2];\
		}else if(pos > ((deg) / 2)){\
			/* insert after middle*/\
			/* extra is inserted as data is being copied to split*/\
			unsigned const self_begin = (deg) / 2 + 1;\
			unsigned const extra_pos = pos - self_begin;\
			memcpy(m[my_split].items,\
				m[self].items + self_begin,\
				extra_pos * sizeof(type));\
			memcpy(m[my_split].items + 1 + extra_pos,\
				m[self].items + pos,\
				((deg) / 2 - (extra_pos + 1)) * sizeof self);\
			m[my_split].items[extra_pos] = extra;\
			/*unsigned j = 0;\
			unsigned i = (deg) / 2 + 1;\
			while(j < extra_pos)\
				m[my_split].items[j++] = m[self].items[i++];\
			m[my_split].items[j++] = extra;\
			while(j < ((deg) / 2))\
				m[my_split].items[j++] = m[self].items[i++];\
			*/\
			memcpy(m[my_split].nodes,\
				m[self].nodes + self_begin,\
				(extra_pos + 1) * sizeof(type));\
			memcpy(m[my_split].nodes + 1 + extra_pos + 1,\
				m[self].nodes + pos + 1,\
				((deg) / 2 - (extra_pos + 1) + 1) * sizeof self);\
			m[my_split].nodes[extra_pos + 1] = sub_split;\
			/*j = 0;\
			i = (deg) / 2 + 1;\
			while(i <= pos)\
				m[my_split].nodes[j++] = m[self].nodes[i++];\
			m[my_split].nodes[j++] = sub_split;\
			while(j <= ((deg) / 2))\
				m[my_split].nodes[j++] = m[self].nodes[i++];\
			*/extra = m[self].items[(deg) / 2];\
		}else{\
			/* insert right in the middle*/\
			/* a plain split with no shifts*/\
			memcpy(m[my_split].items,\
				m[self].items + (deg) / 2,\
				(deg) / 2 * sizeof(type));\
			memcpy(m[my_split].nodes + 1,\
				m[self].nodes + 1 + (deg) / 2,\
				(deg) / 2 * sizeof self);\
			/*for(unsigned j = 0, i = (deg) / 2; j < ((deg) / 2);){\
				m[my_split].items[j++] = m[self].items[i++];\
				m[my_split].nodes[j] = m[self].nodes[i];\
			}*/\
			m[my_split].nodes[0] = sub_split;\
		}\
		m[self].size = (deg) / 2;\
		m[my_split].size = (deg) / 2;\
		sub_split = my_split;\
	}\
	/* bubbled all the way to the top, need to make new root for tree*/\
	unsigned new_root = BNODE_ALLOC(bt);\
	m[new_root].items[0] = extra;\
	m[new_root].nodes[0] = bt->root;\
	m[new_root].nodes[1] = sub_split;\
	m[new_root].size = 1;\
	bt->root = new_root;\
	return 0;\
}
#define BTREE_DEFINE_REMOVE(name, type, cmp, deg)\
static void bnode##name_dealloc(struct bnode##name * nd){\
	nd->size = (deg);\
}\
static void bnode##name_merge_child(struct bnode##name * const restrict m,\
	unsigned const self, unsigned const pos){\
	unsigned const l = m[self].nodes[pos];\
	unsigned const r = m[self].nodes[pos + 1];\
	unsigned const lsiz = m[l].size;\
	unsigned const rsiz = m[r].size;\
	{\
		unsigned i = lsiz;\
		unsigned j = 0;\
		m[l].items[i++] = m[self].items[pos];\
		while(j < rsiz){\
			m[l].items[i] = m[r].items[j];\
			m[l].nodes[i++] = m[r].nodes[j++];\
		}\
		m[l].nodes[i] = m[r].nodes[j];\
	}\
	bnode##name_dealloc(m + r);\
	for(unsigned i = pos, end = m[self].size - 1; i < end;){\
		m[self].items[i] = m[self].items[i + 1];\
		i++;\
		m[self].nodes[i] = m[self].nodes[i + 1];\
	}\
	m[l].size = lsiz + rsiz + 1;\
	assert(m[l].size == (deg) - 1);\
	m[self].size--;\
}\
static int name##_remove(struct name * const bt, type item){\
	struct bnode##name * const m = bt->mem;\
	struct frame{\
		unsigned self;\
		unsigned pos;\
	} stack[64];\
	register unsigned sp = 0;\
	register unsigned self = bt->root;\
	register unsigned pos;\
	struct frame target = {0, 0};\
	if(m[self].size == 0)\
		return 1;\
	while(1){\
		if(target.self)\
			pos = m[self].size;\
		else{\
			register int mid= 0, lo = 0, hi = m[self].size - 1;\
			register int result = 0;\
			while(lo <= hi){\
				mid = (lo + hi) >> 1;\
				result = cmp(item, m[self].items[mid]);\
				if(result < 0)\
					hi = mid - 1;\
				else if(result > 0)\
					lo = mid + 1;\
				else{\
					target.pos = mid;\
					target.self = self;\
					break;\
				}\
			}\
			pos = mid + (result > 0);\
		}\
		sp++;\
		stack[sp].self = self;\
		stack[sp].pos = pos;\
		if(m[self].nodes[0] == 0)\
			break;\
		self = m[self].nodes[pos];\
	}\
	if(!target.self)\
		return 1;\
	/* swap with predecessor*/\
	if(self != target.self)\
		m[target.self].items[target.pos] = m[self].items[pos - 1];\
	else\
		for(unsigned i = pos, end = m[self].size - 1; i < end; i++)\
			m[self].items[i] = m[self].items[i + 1];\
	m[self].size--;\
	if(self == bt->root)\
		return 0;\
	/* fix nodes that are lacking in content*/\
	while(1){\
		if(m[self].size >= (deg) / 2)\
			return 0;\
		sp--;\
		if(!sp)\
			break;\
		self = stack[sp].self;\
		pos = stack[sp].pos;\
		assert(m[m[self].nodes[pos]].size < (deg) / 2);\
		assert(m[self].size < (deg));\
		if(pos == m[self].size){\
			/* lak means lacking, nei means neighbor*/\
			unsigned lak = m[self].nodes[pos];\
			unsigned nei = m[self].nodes[pos - 1];\
			unsigned neisiz = m[nei].size;\
			unsigned laksiz = m[lak].size;\
			assert(laksiz == (deg) / 2 - 1);\
			if(neisiz > (deg) / 2){\
				/* neighbor has spares, sp transfer one to lacking*/\
				m[lak].nodes[laksiz + 1] = m[lak].nodes[laksiz];\
				for(unsigned i = laksiz; i--;){\
					m[lak].items[i + 1] = m[lak].items[i];\
					m[lak].nodes[i + 1] = m[lak].nodes[i];\
				}\
				m[lak].items[0] = m[self].items[pos - 1];\
				m[lak].nodes[0] = m[nei].nodes[neisiz];\
				m[self].items[pos - 1] = m[nei].items[neisiz - 1];\
				m[nei].size--;\
				m[lak].size++;\
			}else{\
				/* combine lack and neighbor*/\
				bnode##name_merge_child(m, self, pos - 1);\
			}\
		}else{\
			unsigned lak = m[self].nodes[pos];\
			unsigned nei = m[self].nodes[pos + 1];\
			unsigned neisiz = m[nei].size;\
			unsigned laksiz = m[lak].size;\
			assert(laksiz == (deg) / 2 - 1);\
			if(neisiz > (deg) / 2){\
				m[lak].items[laksiz] = m[self].items[pos];\
				m[lak].nodes[laksiz + 1] = m[nei].nodes[0];\
				m[self].items[pos] = m[nei].items[0];\
				for(unsigned i = 0; i < neisiz - 1; i++){\
					m[nei].items[i] = m[nei].items[i + 1];\
					m[nei].nodes[i] = m[nei].nodes[i + 1];\
				}\
				m[nei].nodes[neisiz - 1] = m[nei].nodes[neisiz];\
				m[nei].size--;\
				m[lak].size++;\
			}else{\
				bnode##name_merge_child(m, self, pos);\
			}\
		}\
	}\
	if(m[self].size > 0)\
		return 0;\
	if(m[self].nodes[0] == 0)\
		return 0;\
	bt->root = m[self].nodes[0];\
	bnode##name_dealloc(m + self);\
	return 0;\
}
#endif
