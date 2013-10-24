#include "btreev.h"
#include "stdlib.h"
#include "string.h"
struct bnode{
	unsigned size;
	unsigned in;
};
struct btree{
	unsigned deg;
	unsigned ele_size;
	unsigned in_size;
	unsigned lf_size;
	bt_cmp cmp;
	struct bnode * root;
};
static void * items(struct btree const * bt, struct bnode * nd, unsigned i)
{
	return ((char*)(nd + 1)) + (bt->ele_size * i);
}
static struct bnode ** nodes(struct btree const * bt, struct bnode * nd, unsigned i)
{
	return ((struct bnode**)(((char*)nd) + bt->lf_size)) + i;
}
struct btree * btree_new(unsigned ele_size, unsigned deg, bt_cmp cmp)
{
	struct btree * bt = calloc(1, sizeof *bt);
	bt->ele_size = ele_size;
	bt->deg = deg >= 4 ? deg & ~1 : 4;
	bt->cmp = cmp;
	bt->lf_size = sizeof(struct bnode) + ((bt->deg - 1) * ele_size);
	bt->in_size = bt->lf_size + (bt->deg * sizeof(void*));
	bt->root = calloc(1, bt->lf_size);
	return bt;
}
static void bnode_del_rec(struct btree * bt, struct bnode * nd)
{
	if(nd->in)
	{
		for(unsigned i = 0; i <= nd->size; i++)
			bnode_del_rec(bt, *nodes(bt, nd, i));
	}
	free(nd);
}
void btree_del(struct btree * bt)
{
	bnode_del_rec(bt, bt->root);
	free(bt);
}
void const * btree_get(struct btree const *  bt, void const * item)
{
	struct bnode * nd = bt->root;
	while(1)
	{
		int mid = 0;
		for(int lo = 0, hi = nd->size - 1; lo <= hi;)
		{
			mid = (lo + hi) / 2;
			int result = (*bt->cmp)(item, items(bt, nd, mid));
			if(result < 0)
				hi = mid - 1;
			else if(result > 0)
				lo = ++mid;
			else
				return items(bt, nd, mid);
		}
		if(nd->in)
			nd = *nodes(bt, nd, mid);
		else
			return NULL;
	}
}

static struct bnode * bnode_split(struct btree *  bt, struct bnode * cur,
	struct bnode * par, unsigned pos)
{
	struct bnode * split = calloc(1, cur->in ? bt->in_size : bt->lf_size);
	memcpy(items(bt, split, 0), items(bt, cur, bt->deg / 2),
		(bt->deg / 2 - 1) * bt->ele_size);
	if(cur->in)
	{
		memcpy(nodes(bt, split, 0), nodes(bt, cur, bt->deg / 2),
			(bt->deg / 2) * sizeof cur);
		split->in = 1;
	}
	memmove(items(bt, par, pos + 1), items(bt, par, pos),
		(par->size - pos) * bt->ele_size);
	memmove(nodes(bt, par, pos + 2), nodes(bt, par, pos + 1),
		(par->size - pos) * sizeof cur);

	memcpy(items(bt, par, pos), items(bt, cur, bt->deg / 2 - 1),
		bt->ele_size);
	*nodes(bt, par, pos + 1) = split;
	par->size++;
	split->size = bt->deg / 2 - 1;
	cur->size = bt->deg / 2 - 1;
	return split;
}
int btree_insert(struct btree *  bt, void const *  item)
{
	struct bnode * cur = bt->root;
	struct bnode * par = NULL;
	if(cur->size == (bt->deg - 1))
	{
		par = calloc(1, bt->in_size);
		par->in = 1;
		*nodes(bt, par, 0) = bt->root;
		bt->root = par;
	}
	unsigned pos = 0;
	while(1)
	{
		unsigned mid = 0;
		for(unsigned lo = 0, hi = cur->size - 1; (int)lo <= (int)hi;)
		{
			mid = (lo + hi) / 2;
			int result = (*bt->cmp)(item, items(bt, cur, mid));
			if(result < 0)
				hi = mid - 1;
			else if(result > 0)
				lo = ++mid;
			else
				return 1;
		}
		if(cur->size == bt->deg - 1)
		{
			struct bnode * split = bnode_split(bt, cur, par, pos);
			if(mid >= (bt->deg / 2))
			{
				mid -= bt->deg / 2;
				cur = split;
			}
		}
		if(cur->in)
		{/* this is kind of like recursion*/
			par = cur;
			cur = *nodes(bt, cur, mid);
			pos = mid;
		}
		else
		{
			memmove(items(bt, cur, mid + 1), items(bt, cur, mid),
				(cur->size - mid) * bt->ele_size);
			memcpy(items(bt, cur, mid), item, bt->ele_size);
			cur->size++;
			return 0;
		}
	}
}

static void bnode_merge(struct btree * bt, struct bnode * cur, unsigned pos)
{
	struct bnode * l = *nodes(bt, cur, pos);
	struct bnode * r = *nodes(bt, cur, pos + 1);
	memcpy(items(bt, l, bt->deg / 2), items(bt, r, 0),
		(bt->deg / 2 - 1) * bt->ele_size);
	if(l->in)
		memcpy(nodes(bt, l, bt->deg / 2), nodes(bt, r, 0),
			(bt->deg / 2) * sizeof cur);
	memcpy(items(bt, l, bt->deg / 2 - 1), items(bt, cur, pos), bt->ele_size);
	memmove(items(bt, cur, pos), items(bt, cur, pos + 1),
		(cur->size - pos - 1) * bt->ele_size);
	memmove(nodes(bt, cur, pos + 1), nodes(bt, cur, pos + 2),
		(cur->size - pos - 1) * sizeof cur);
	free(r);
	l->size = bt->deg - 1;
	cur->size--;
}
static void bnode_stealr(struct btree *  bt, struct bnode *  cur,
	struct bnode *  next, struct bnode *  right, unsigned pos)
{
	memcpy(items(bt, next, bt->deg / 2 - 1), items(bt, cur, pos), bt->ele_size);
	memcpy(items(bt, cur, pos), items(bt, right, 0), bt->ele_size);
	if(next->in)
	{
		*nodes(bt, next, bt->deg / 2) = *nodes(bt, right, 0);
		memmove(nodes(bt, right, 0), nodes(bt, right, 1),
			right->size * sizeof cur);
	}
	memmove(items(bt, right, 0), items(bt, right, 1),
		(right->size - 1) * bt->ele_size);
	right->size--;
	next->size++;
}
static void bnode_steall(struct btree *  bt, struct bnode *  cur,
	struct bnode *  next, struct bnode *  left, unsigned pos)
{
	memmove(items(bt, next, 1), items(bt, next, 0),
		(bt->deg / 2 - 1) * bt->ele_size);
	memcpy(items(bt, next, 0), items(bt, cur, pos - 1), bt->ele_size);
	if(next->in)
	{
		memmove(nodes(bt, next, 1), nodes(bt, next, 0),
			(bt->deg / 2) * sizeof cur);
		*nodes(bt, next, 0) = *nodes(bt, left, left->size);
	}
	memcpy(items(bt, cur, pos - 1), items(bt, left, left->size - 1),
		bt->ele_size);
	left->size--;
	next->size++;
}
int btree_remove(struct btree * bt, void const * item)
{
	struct bnode * cur = bt->root;
	unsigned pos = 0;
	struct bnode * targ_cur = NULL;
	unsigned targ_pos = 0;
	while(1)
	{
		if(!targ_cur)
		{
			int mid = 0;
			for(int lo = 0, hi = cur->size - 1; lo <= hi;)
			{
				mid = (lo + hi) / 2;
				int result = (*bt->cmp)(item, items(bt, cur, mid));
				if(result < 0)
					hi = mid - 1;
				else if(result > 0)
					lo = ++mid;
				else{
					targ_pos = mid;
					targ_cur = cur;
					break;
				}
			}
			pos = mid;
		}
		else
		{
			pos = cur->size;
		}
		if(!cur->in)
			break;
		struct bnode * next = *nodes(bt, cur, pos);
		if(next->size == bt->deg / 2 - 1)
		{
			if(pos != cur->size)
			{
				struct bnode * right = *nodes(bt, cur, pos + 1);
				if(right->size != bt->deg / 2 - 1)
					bnode_stealr(bt, cur, next, right, pos);
				else
					bnode_merge(bt, cur, pos);
				if(cur == targ_cur)
					targ_cur = 0;
			}
			else
			{
				struct bnode * left = *nodes(bt, cur, pos - 1);
				if(left->size != bt->deg / 2 - 1)
				{
					bnode_steall(bt, cur, next, left, pos);
				}
				else
				{
					bnode_merge(bt, cur, pos - 1);
					next = left;
				}
			}
		}
		cur = next;
	}
	if(targ_cur)
	{
	/* swap with predecessor*/
		if(cur == targ_cur)
		{
			memmove(items(bt, cur, pos), items(bt, cur, pos + 1),
				(cur->size - 1 - pos) * bt->ele_size);
		}
		else
		{
			memcpy(items(bt, targ_cur, targ_pos),
				items(bt, cur, pos - 1),
				bt->ele_size);
		}
		cur->size--;
		if(bt->root->size == 0)
		{
			if(bt->root->in)
			{
				struct bnode * new_root = *nodes(bt, bt->root, 0);
				free(bt->root);
				bt->root = new_root;
			}
		}
		return 0;
	}
	else
		return 1;
}
