#ifndef RBTREE_H
#define RBTREE_H
struct rb_node{
	struct rb_node * left;
	struct rb_node * right;
	unsigned long is_red;
};
typedef int (* rb_cmp)(void const *, void const *);
struct rbtree{
	struct rb_node * root;
	rb_cmp cmp;
	int offset;
};
#define rb_offsetof(type, field)\
( ((char*)&((type*)NULL)->field) - ((char*)NULL) )

#define rb_init(rb, cmp_ptr, type, field)\
do{\
	(rb)->root = NULL;\
	(rb)->cmp = (cmp_ptr);\
	(rb)->offset = rb_offsetof(type, field);\
}while(0)

void * rb_find(struct rbtree const * rb, void const * item);
int rb_insert(struct rbtree * rb, void * item);
int rb_remove(struct rbtree * rb, void * item);
#endif
