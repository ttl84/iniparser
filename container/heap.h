#ifndef HEAP_H
#define HEAP_H
#include <stddef.h>
enum min_max{
	HEAP_MIN,
	HEAP_MAX
};

/* a compare function
returns:
- 0 for equal
- 1 for greater than
- (-1) for lesser than
*/
typedef int (* heap_cmp)(void const * mem1, void const * mem2);

/* this is the structure that contains the heap.*/
struct heap;

struct heap * heap_new(heap_cmp cmp, enum min_max type);

void heap_del(struct heap *);
/* clears all elements in the heap.*/
void heap_clear(struct heap * hep);

/* adds an item to the end of the heap.*/
int heap_push(struct heap * hep, void * nde);

/* inserts item into heap.*/
int heap_insert(struct heap * hep, void * nde);

/* removes the top item from heap.*/
int heap_remove_top(struct heap * hep);

/* return pointer to root*/
void * heap_get_root(struct heap * hep);

/* restores heap property downwards (towards leaf)*/
void heapify_down(struct heap * hep, size_t from);

/* restores heap property upwards (towards root)*/
void heapify_up(struct heap * hep, size_t from);

/* restores heap property of a very messed up heap*/
void heapify(struct heap * hep);


#endif
