#ifndef ARRAY_H
#define ARRAY_H
#include <stddef.h>
struct array;

/* creates an array*/
struct array * array_new(size_t bytes);

/* deletes an array*/
void array_del(struct array *);

/* resizes the array to bytes*/
int array_resize(struct array *, size_t bytes);

/* makes sure array is at least bytes long*/
int array_reserve(struct array * ray, size_t bytes);

/* get the size of the array, in bytes.*/
size_t array_size(struct array *);

/* get the head of the array*/
void * array_head(struct array *);
#endif
