#include "hashsetv.h"
#include "stdlib.h"
#include "string.h"
#define mod_pow2(i, n)\
((i) & ((n) - 1))

struct entry{
	struct entry * next;
	char item[1];
};
struct hashset{
	unsigned size;
	unsigned ele_size;
	unsigned nitem;
	struct entry ** buk;
	hs_cmp cmp;
	hs_hash hash;
	
};
struct hashset * hashset_new(unsigned ele_size, hs_cmp cmp, hs_hash hash)
{
	struct hashset * hs = calloc(1, sizeof *hs);
	if(hs)
	{
		hs->ele_size = ele_size;
		hs->size = 1;
		hs->nitem = 0;
		hs->buk = calloc(hs->size, sizeof(void*));
		if(hs->buk == NULL)
		{
			free(hs);
			hs = NULL;
		}
		hs->hash = hash;
		hs->cmp = cmp;
	}
	return hs;
}
void hashset_del(struct hashset * hs)
{
	if(hs != NULL)
	{
		struct entry ** buk = hs->buk;
		for(unsigned i = hs->size; i--;)
		{
			while(buk[i])
			{
				struct entry * deleted = buk[i];
				buk[i] = deleted->next;
				free(deleted);
			}
		}
		free(buk);
		hs->buk = NULL;
		hs->nitem = 0;
		free(hs);
	}
}
void * hashset_get(struct hashset * hs, void const * item)
{
	unsigned i = mod_pow2((*hs->hash)(item), hs->size);
	struct entry * iter = hs->buk[i];
	while(iter)
	{
		if(0 == (*hs->cmp)(item, iter->item))
			return iter->item;
		iter = iter->next;
	}
	return NULL;
}
static int grow(struct hashset * hs)
{
	struct hashset bigger = *hs;
	bigger.size *= 2;
	if(bigger.size > hs->size)
	{
		bigger.buk = calloc(bigger.size, sizeof(void*));
		if(bigger.buk != NULL)
		{
			for(unsigned i = 0; i < hs->size; i++)
			{
				while(hs->buk[i])
				{
					struct entry * top = hs->buk[i];
					hs->buk[i] = top->next;
					unsigned hash = (*hs->hash)(top->item);
					hash = mod_pow2(hash, bigger.size);
					top->next = bigger.buk[hash];
					bigger.buk[hash] = top;
				}
			}
			*hs = bigger;
			return 0;
		}
	}
	return -1;
}
int hashset_insert_s(struct hashset * hs, void const * item, unsigned size)
{
	if(hs->nitem * 2 < hs->size)
	{
		struct entry * new_node = calloc(1, sizeof(void*) + size);
		unsigned i = mod_pow2((*hs->hash)(item), hs->size);
		memcpy(new_node->item, item, size);
		new_node->next = hs->buk[i];
		hs->buk[i] = new_node;
		hs->nitem++;
		return 0;
	}
	else
	{
		if(grow(hs) == 0)
			return hashset_insert_s(hs, item, size);
		else
			return 1;
	}
}
int hashset_insert(struct hashset * hs, void const * item)
{
	return hashset_insert_s(hs, item, hs->ele_size);
}
int hashset_remove(struct hashset * hs, void const * item)
{
	unsigned i = mod_pow2((*hs->hash)(item), hs->size);
	struct entry * iter = hs->buk[i];
	struct entry head = {.next = iter};
	struct entry * prev = &head;
	while(iter)
	{
		if(0 != (*hs->cmp)(item, iter->item))
		{
			prev = iter;
			iter = iter->next;
		}
		else
		{
			prev->next = iter->next;
			hs->buk[i] = head.next;
			free(iter);
			hs->nitem--;
			return 0;
		}
	}
	return -1;
}

