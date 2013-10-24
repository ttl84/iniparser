#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
struct ht_pair{
	void * key;
	void * val;
	struct ht_pair * next;
};
struct ht{
	struct ht_pair ** buks;
	ht_cmp cmp;
	ht_hash hash;
	size_t size;
	size_t nfilled;
	size_t nitem;
};
static size_t round_up(size_t n)
{
	size_t p = 1;
	while(p < n)
		p *= 2;
	return p;
}
static size_t reduce(size_t i, size_t n)
{
	return i & (n - 1);
}
size_t str_hash(void const * str)
{
	size_t sum = 0;
	for(char const * s = str; *s != 0; s++)
		sum = sum * 257  + *s;
	return sum;
}
int str_cmp(void const * str1, void const * str2)
{
	return strcmp(str1, str2);
}
struct ht_pair * pair_new(void * key, void * val)
{
	struct ht_pair * new_pair = malloc(sizeof *new_pair);
	if(new_pair != NULL)
	{
		new_pair->key = key;
		new_pair->val = val;
	}
	return new_pair;
}
void pair_del(struct ht_pair * pair)
{
	free(pair);
}
struct ht * ht_new(size_t size, ht_cmp cmp_ptr, ht_hash hash_ptr)
{
	size = round_up(size);
	struct ht * table = malloc(sizeof *table);
	if(table == NULL)
		goto malloc_error1;

	struct ht_pair ** buks = calloc(size, sizeof(void *));
	if(buks == NULL)
		goto malloc_error2;

	table->buks = buks;
	table->size = size;
	table->nfilled = 0;
	table->nitem = 0;
	table->cmp = cmp_ptr;
	table->hash = hash_ptr;
	return table;

malloc_error2:
	free(table);
malloc_error1:
	return NULL;
}
void ht_del(struct ht * table)
{
	if(table == NULL)
		return;
	int i = 0;
	struct ht_pair ** buks = table->buks;
	while(table->nitem > 0)
	{
		if(buks[i] == NULL)
		{
			++i;
		}
		else
		{
			struct ht_pair * deleted = buks[i];
			buks[i] = deleted->next;
			pair_del(deleted);
			table->nitem--;
		}
	}
	free(table);
}
bool ht_haskey(struct ht * table, void const * key)
{
	size_t i = reduce((*table->hash)(key), table->size);
	struct ht_pair * iter = table->buks[i];
	while(iter != NULL)
	{
		if(0 == (*table->cmp)(key, iter->key))
			return true;
		iter = iter->next;
	}
	return false;
}
void * ht_get(struct ht * table, void const * key)
{
	size_t i = reduce((*table->hash)(key), table->size);
	struct ht_pair * iter = table->buks[i];
	while(iter != NULL)
	{
		if(0 == (*table->cmp)(key, iter->key))
			return iter->val;
		iter = iter->next;
	}
	return NULL;
}
int ht_insert(struct ht * table, void * key, void * val)
{
	struct ht_pair * pair = pair_new(key, val);
	if(pair == NULL)
		return 1;
	size_t i = reduce((*table->hash)(key), table->size);	
	pair->next = table->buks[i];
	table->buks[i] = pair;
	table->nitem++;
	if(pair->next == NULL)
		table->nfilled++;
	return 0;
}
int ht_remove(struct ht * table, void const * key, void ** retkey, void ** retval)
{		
	size_t i = reduce((*table->hash)(key), table->size);	
	struct ht_pair * iter = table->buks[i];
	struct ht_pair head = {.next = iter};
	struct ht_pair * prev = &head;
	/* search for equal key*/
	while(iter != NULL)
	{
		if(0 != (*table->cmp)(key, iter->key))
		{
			prev = iter;
			iter = iter->next;
		}
		else
			break;
	}
	/* fail if nothing to return*/
	if(iter == NULL)
		return 1;

	/* remove the pair from table*/
	prev->next = iter->next;
	table->buks[i] = head.next;

	/* retrieve key and vl*/
	if(retkey != NULL)
		*retkey = iter->key;
	if(retval != NULL)
		*retval = iter->val;
	pair_del(iter);

	/* update table info*/
	table->nitem--;
	if(table->buks[i] == NULL)
		table->nfilled--;
	return 0;
}
int ht_grow(struct ht * table)
{
	size_t old_size = table->size;
	size_t new_size = old_size * 2;
	if(new_size <= old_size)
		return 1;

	struct ht_pair ** old_buks = table->buks;
	struct ht_pair ** new_buks = calloc(new_size, sizeof(void*));
	/* failed to allocate bigger array*/
	if(new_buks == NULL)
		return 2;

	ht_hash hash = table->hash;
	size_t i = 0;
	while(i < old_size)
	{
		/* find a non empty bucket*/
		if(old_buks[i] == NULL)
		{
			i++;
		}
		else
		{
			/* pop the bucket*/
			struct ht_pair * pair = old_buks[i];
			old_buks[i] = pair->next;
			pair->next = NULL;

			/* rehash the key*/
			size_t i2 = reduce((*hash)(pair->key), new_size);
			struct ht_pair head = {.next = new_buks[i2]};
			struct ht_pair * iter = &head;
			while(iter->next != NULL)
				iter = iter->next;
			iter->next = pair;
			new_buks[i2] = head.next;
		}
	}
	free(old_buks);
	table->size = new_size;
	table->buks = new_buks;
	return 0;
}
size_t ht_nitem(struct ht * table)
{
	return table->nitem;
}
size_t ht_nfilled(struct ht * table)
{
	return table->nfilled;
}
size_t ht_size(struct ht * table)
{
	return table->size;
}
struct ht_iter{
	struct ht_pair * current;
	struct ht_pair * next;
	struct ht * table;
	size_t i;
};
struct ht_iter * ht_iter_new(void)
{
	struct ht_iter * new_iter = calloc(1, sizeof *new_iter);
	return new_iter;
}
struct ht_iter * ht_iter_init(struct ht_iter * iter, struct ht * table)
{
	iter->table = table;
	iter->current = NULL;
	iter->next = table->buks[0];
	iter->i = 0;
	return iter;	
}
void ht_iter_del(struct ht_iter * iter)
{
	free(iter);
}
void * ht_key(struct ht_iter * iter)
{
	if(iter->current == NULL)
		return NULL;
	return iter->current->key;
}
void * ht_val(struct ht_iter * iter)
{
	if(iter->current == NULL)
		return NULL;
	return iter->current->val;
}
bool ht_next(struct ht_iter * iter)
{
	struct ht const * const table = iter->table;
	struct ht_pair * const * const buks = table->buks;
	size_t const size = table->size;
	if(iter->i == size)
		return false;
	while(iter->next == NULL)
	{
		iter->i++;
		if(iter->i == size)
			return false;
		if(buks[iter->i] != NULL)
			iter->next = buks[iter->i];
	}	
	iter->current = iter->next;
	iter->next = iter->current->next;
	return true;
}
