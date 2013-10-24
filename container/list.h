#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED
/* this is the head of a linked list node*/
struct ll{
	struct ll * next;
	struct ll * prev;
	void * item;
};

/* make a new_node whose item will be item*/
struct ll * ll_new(void * item);

/* deletes a linked list  node*/
void ll_del(struct ll * old);

/* every linked list head shoud be inited first.*/
struct ll * ll_init(struct ll * node);

/* insert node inbetween prev and next.*/
struct ll * ll_insert(struct ll * prev, struct ll * node, struct ll * next);

/* insert node as the next of a linked list*/
#define ll_pshr(head, node)\
ll_insert((head), (node), (head)->next)

/* insert node as the prev of a linked list*/
#define ll_pshl(head, node)\
ll_insert((head)->left, (node), (head))

/* remove a node from a linked list.*/
struct ll * ll_remove(struct ll * node);

/* remove the next of a linked list*/
#define ll_popr(head)\
ll_remove((head)->next)

/* remove the prev of a linked list*/
#define ll_popl(head)\
ll_remove((head)->prev)

#define ll_for(itername, begin, end)\
for(struct ll * itername = (begin), * nextnode = itername->next; itername != &(end); itername = nextnode, nextnode = nextnode->next)
#endif
