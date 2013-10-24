#ifndef HASHGRAPH_H
#define HASHGRAPH_H
#include "hashtable.h"
#define edge_t(name)\
name##_half_edge

#define HASHGRAPH_MAKE(name, type, cmp, hash)\
typedef struct edge_t(name){\
	unsigned weight;\
	type v;\
	struct edge_t(name) * next;\
} edge_t(name);\
HASHTABLE_MAKE(name, type, edge_t(name) *, cmp, hash)\
static edge_t(name) * name##_getedges(struct name * g, type v)\
{\
	if(name##_haskey(g, v))\
		return name##_getval(g, v);\
	else\
		return NULL;\
}\
static int name##_hasedge(struct name * g, type v1, type v2)\
{\
	if(name##_haskey(g, v1))\
	{\
		edge_t(name) * iter = name##_getval(g, v1);\
		while(iter)\
		{\
			if(cmp(v2, iter->v) == 0)\
				return 1;\
			iter = iter->next;\
		}\
		return 0;\
	}\
	else\
		return 0;\
}\
static int name##_addedge(struct name * g, type v1, type v2, unsigned weight)\
{\
	edge_t(name) * old_top = NULL;\
	if(name##_haskey(g, v1))\
	{\
		old_top = name##_getval(g, v1);\
		name##_delkey(g, v1);\
	}\
	edge_t(name) * edge = calloc(1, sizeof *edge);\
	edge->weight = weight;\
	edge->v = v2;\
	edge->next = old_top;\
	name##_setval(g, v1, edge);\
	return 0;\
}\
static int name##_deledge(struct name * g, type v1, type v2)\
{\
	if(name##_haskey(g, v1))\
	{\
		edge_t(name) * iter = name##_getval(g, v1);\
		edge_t(name) head = {.next = iter};\
		edge_t(name) * prev = &head;\
		while(iter)\
		{\
			if(cmp(v2, iter->v) == 0)\
			{\
				prev->next = iter->next;\
				free(iter);\
				if(prev == &head)\
				{\
					name##_delkey(g, v1);\
					if(head.next)\
						name##_setval(g, v1, head.next);\
				}\
				return 0;\
			}\
			prev = iter;\
			iter = iter->next;\
		}\
		return 1;\
	}\
	else\
		return 1;\
}
#endif
