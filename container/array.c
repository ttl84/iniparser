#include "array.h"
#include <stdlib.h>
struct array{
	size_t size;
	void * buf;
};
struct array * array_new(size_t bytes)
{
	struct array * new_array = malloc(sizeof *new_array);
	if(new_array == NULL)
		goto malloc_err1;
	
	new_array->buf = malloc(bytes);
	if(new_array->buf == NULL)
		goto malloc_err2;
	
	new_array->size = bytes;
	return new_array;
malloc_err2:
	free(new_array);
malloc_err1:
	return NULL;
}
void array_del(struct array * ray)
{
	if(ray == NULL)
		return;
	free(ray->buf);
	ray->buf = NULL;
	ray->size = 0;
	free(ray);
}
int array_resize(struct array * ray, size_t bytes)
{
	if(ray->size == bytes)
		return 0;
	void * resized = realloc(ray->buf, bytes);
	if(resized != NULL)
	{
		ray->buf = resized;
		ray->size = bytes;
		return 0;
	}
	return 1;
}
int array_reserve(struct array * ray, size_t bytes)
{
	if(bytes > ray->size)
		return array_resize(ray, bytes);
	return 0;
}
size_t array_size(struct array * ray)
{
	return ray->size;
}
void * array_head(struct array * ray)
{
	return ray->buf;
}
