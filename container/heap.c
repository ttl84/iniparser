#include "heap.h"
#include "array.h"
#include <stdlib.h>
#include <stdio.h>
struct heap{
	struct array * array;
	size_t size;
	heap_cmp cmp;
	int lt;
};
/* calculates the index of parent node of a child node at index i*/
static inline size_t heap_get_parent(size_t i)
{
	return (i - 1) / 2;
}
/* calculates the index of the left chid*/
static inline size_t heap_get_left(size_t i)
{
	return 2 * i + 1;
}
/* calculates the index of the right child*/
static inline size_t heap_get_right(size_t i)
{
	return 2 * i + 2;
}

struct heap * heap_new(heap_cmp cmp, enum min_max type)
{
	struct heap * hep = malloc(sizeof *hep);
	if(hep == NULL)
		goto malloc_err;

	hep->array = array_new(4 * sizeof(void*));
	if(hep->array == NULL)
		goto array_err;
	
	/* comparison function defaults to memcmp*/
	if(cmp == NULL)
		goto input_err;
	hep->cmp = cmp;
	
	/* heap type*/
	if(type == HEAP_MIN)
		hep->lt = -1;
	else if(type == HEAP_MAX)
		hep->lt = 1;
	else
		goto input_err;
	
	hep->size = 0;

	return hep;
input_err:
array_err:
	free(hep);
malloc_err:
	return NULL;
}
void heap_del(struct heap * hep)
{
	array_del(hep->array);
	free(hep);
}
int heap_push(struct heap * hep, void * ptr)
{
	/* make sure array is long enough*/
	array_reserve(hep->array, (hep->size + 1) * sizeof(void*));
	
	/* appends new pointer to item to the end of array*/
	void ** array = array_head(hep->array);
	array[hep->size] = ptr;
	hep->size++;
	return 0;
}
int heap_insert(struct heap * hep, void * nde)
{
	if(heap_push(hep, nde) != 0)
		return 1;
	
	/* restore heap starting from last*/
	heapify_up(hep, hep->size - 1);
	return 0;
}
int heap_remove_top(struct heap * hep)
{
	/* return fail if heap is already empty*/
	if(hep->size == 0)
		return 1;

	/* set root item to the last item*/
	void ** array = array_head(hep->array);
	hep->size--;
	array[0] = array[hep->size];

	/* restore heap starting from root*/
	heapify_down(hep, 0);
	return 0;
}
void heapify_down(struct heap * hep, size_t root)
{
	void ** const array = array_head(hep->array);
	heap_cmp const cmp = hep->cmp;
	int const lt = hep->lt;
	int const gt = (lt == -1 ? 1 : -1);
	
	while(heap_get_left(root) < hep->size)
	{
		size_t left = heap_get_left(root);
		size_t right = heap_get_right(root);
		
		/* get the correct next*/
		size_t next = left;
		if(right < hep->size && (*cmp)(array[left], array[right]) == gt)
			next = right;

		/* swap root and child if needed*/
		if((*cmp)(array[root], array[next]) == gt)
		{
			void * tmp = array[root];
			array[root] = array[next];
			array[next] = tmp;
		}
		/* if not then stop*/
		else
		{
			break;
		}
		
		/* go to the next level*/
		root = next;
	}
}
void heapify_up(struct heap * hep, size_t child)
{
	void ** const array = array_head(hep->array);
	heap_cmp const cmp = hep->cmp;
	int const lt = hep->lt;
	int const gt = (lt == -1 ? 1 : -1);
	
	size_t parent = heap_get_parent(child);
	while(parent != child &&
		(*cmp)(array[parent], array[child]) == gt)
	{
		/* swap parent and child because they dont have the right order*/
		void * tmp = array[parent];
		array[parent] = array[child];
		array[child] = tmp;

		/* go up one level*/
		child = parent;
		parent = heap_get_parent(parent);
	}
}
void heapify(struct heap * hep)
{
	size_t i = hep->size;
	while(1)
	{
		heapify_down(hep, i);
		if(i == 0)
			break;
		i--;
	}
}
void * heap_get_root(struct heap * hep)
{
	if(hep->size == 0)
		return NULL;
	void ** array = array_head(hep->array);
	return array[0];
}
void heap_clear(struct heap * hep)
{
	hep->size = 0;
}
