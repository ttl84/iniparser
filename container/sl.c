#include "sl.h"
#include <stdlib.h>
struct sl * sl_new(void * item, struct sl * next)
{
	struct sl * new = calloc(1, sizeof *new);
	new->item = item;
	new->next = next;
	return new;
}
void sl_del(struct sl * nd)
{
	free(nd);
}
struct sl * sl_cpy_rec(struct sl * iter)
{
	if(iter == NULL)
		return NULL;
	else
		return sl_new(iter->item, sl_cpy_rec(iter->next));
}
void sl_del_rec(struct sl * iter)
{
	while(iter)
	{
		struct sl * next = iter->next;
		sl_del(iter);
		iter = next;
	}
}
struct sl * sl_reversed(struct sl * iter)
{
	struct sl * reversed = NULL;
	while(iter)
	{
		struct sl * tail = iter->next;
		iter->next = reversed;
		reversed = iter;
		iter = tail;
	}
	return reversed;
}
struct sl * sl_cat(struct sl * l0, struct sl * l1)
{
	if(l0 == NULL)
		return l1;
	struct sl * iter = l0;
	while(iter->next)
		iter = iter->next;
	iter->next = l1;
	return l0;
}

struct sl * sl_remove(struct sl * iter, void * item)
{
	struct sl head = {.next = iter};
	struct sl * prev = &head;
	while(iter)
	{
		if(iter->item == item)
		{
			prev->next = iter->next;
			sl_del(iter);
			break;
		}
		else
		{
			prev = iter;
			iter = iter->next;
		}
	}
	return head.next;
}
struct sl * sl_sort(struct sl * iter, int (*cmp)(void const *, void const * ))
{
	if(iter == NULL || iter->next == NULL)
		return iter;
	struct sl * less = NULL;
	struct sl * more = NULL;
	struct sl * same = NULL;
	struct sl * pivot = iter;
	iter = iter->next;
	while(iter)
	{
		struct sl * top = iter;
		iter = iter->next;
		int result = (*cmp)(top->item, pivot->item);
		if(result < 0)
		{
			top->next = less;
			less = top;
		}
		else if(result > 0)
		{
			top->next = more;
			more = top;
		}
		else
		{
			top->next = same;
			same = top;
		}
	}
	more = sl_sort(more, cmp);
	less = sl_sort(less, cmp);

	pivot->next = same;
	pivot = sl_cat(pivot, more);

	less = sl_cat(less, pivot);
	return less;
}
