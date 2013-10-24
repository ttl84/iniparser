#include "list.h"
#include <stdlib.h>
struct ll * ll_new(void * item)
{
	struct ll * new_node = malloc(sizeof *new_node);
	if(new_node != NULL)
	{
		ll_init(new_node);
		new_node->item = item;
	}
	return new_node;
}
void ll_del(struct ll * node)
{
	free(node);
}
struct ll * ll_init(struct ll * node)
{
	node->item = NULL;
	node->prev = node;
	node->next = node;
	return node;
}
struct ll * ll_insert(struct ll * prev, struct ll * node, struct ll * next)
{
	prev->next = node;
	next->prev = node;
	node->prev = prev;
	node->next = next;
	return node;
}
struct ll * ll_remove(struct ll * node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = node;
	node->prev = node;
	return node;
}
