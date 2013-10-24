#include "set.h"
#include <stdlib.h>
struct node{
	void * item;
	struct node * next;
} tail = {
	.item = NULL,
	.next = NULL
};
struct set{
	struct node * list;
	set_cmp cmp;
	set_cpy cpy;
	set_dtor dtor;
	size_t card;
};
struct set * set_new(set_cmp cmp, set_cpy cpy, set_dtor dtor)
{
	struct set * new_set = malloc(sizeof *new_set);
	*new_set = (struct set){
		.list = &tail,
		.cmp = cmp,
		.cpy = cpy,
		.dtor = dtor,
		.card = 0
	};
	return new_set;
}
void set_del(struct set * s)
{
	
}
int set_insert(struct set * s, void const * item)
{
	set_cmp cmp = s->cmp;
	set_cpy cpy = s->cpy;
	struct node head = {.next = s->list};
	struct node * prev = &head;
	struct node * iter = head.next;
	while(iter != &tail)
	{
		int result = (*cmp)(item, iter->item);
		if(result == 0)
			return 1;
		else if(result < 0)
			break;
		prev = iter;
		iter = iter->next;
	}
	struct node * new_node = malloc(sizeof *new_node);
	*new_node = (struct node){
		.item = (*cpy)(item),
		.next = iter
	};
	prev->next = new_node;
	s->list = head.next;
	return 0;
}
int set_remove(struct set * s, void const * item)
{
	set_cmp cmp = s->cmp;
	set_dtor dtor = s->dtor;
	struct node head = {.next = s->list};
	struct node * prev = &head;
	struct node * iter = head.next;
	while(1)
	{
		if(iter == &tail)
			return 1;
		int result = (*cmp)(item, iter->item);
		if(result == 0)
			break;
		else if(result > 0)
			return 1;
		prev = iter;
		iter = iter->next;
	}
		
	prev->next = iter->next;
	s->list = head.next;
	(*dtor)(iter->item);
	free(iter);
	return 0;
}