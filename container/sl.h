#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H
struct sl{
	struct sl * next;
	void * item;
};
struct sl * sl_new(void * item, struct sl * link);
void sl_del(struct sl *);

struct sl * sl_cpy_rec(struct sl *);
void sl_del_rec(struct sl *);

struct sl * sl_reversed(struct sl * list);
struct sl * sl_cat(struct sl * l0, struct sl * l1);

struct sl * sl_remove(struct sl * list, void * item);

struct sl * sl_sort(struct sl * head, int (*cmp)(void const *, void const *));
#define sl_for(ITER, HEAD)\
	for(struct sl * ITER = (HEAD); ITER; ITER = ITER->next)
#endif
