#include "rbtree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define IS_RED(nodeptr) ((nodeptr) != 0 && (nodeptr)->is_red != 0)
#define IS_BLACK(nodeptr) ((nodeptr) == 0 || (nodeptr)->is_red == 0)
#define IS_3(nodeptr) ((nodeptr) != 0 && IS_RED((nodeptr)->right))
#define IS_2(nodeptr) ((nodeptr) == 0 || IS_BLACK((nodeptr)->right))
static inline
struct rb_node * rb_init(struct rb_node * node)
{
	node->left = 0;
	node->right = 0;
	node->is_red = 1;
	return node;
}
static inline
struct rb_node * rb_init_root(struct rb_node * node)
{
	node->left = 0;
	node->right = 0;
	node->is_red = 0;
	return node;
}
static inline
struct rb_node * rb_set_black(struct rb_node * const node)
{
	if(node == 0)
		return 0;
	node->is_red = 0;
	return node;
}
static inline
struct rb_node * rb_set_red(struct rb_node * const node)
{
	if(node == 0)
		return 0;
	node->is_red = 1;
	return node;
}
static inline
int default_cmp(void const * addr1, void const * addr2)
{
	return (char*)addr1 - (char*)addr2;
}
static inline
int compare(struct rb_node const * addr1, struct rb_node const * addr2,
			rb_cmp cmp, int offset)
{
	if(cmp == 0)
		return default_cmp(addr1, addr2);
	return (*cmp)((char*)addr1 - offset, (char*)addr2 - offset);
}
static inline
struct rb_node * rb_ror(struct rb_node * node, struct rb_node * parent)
{
	struct rb_node * pivot = node->left;
	node->left = pivot->right;
	pivot->right = node;
	if(parent != 0)
	{
		if(parent->left == node)
			parent->left = pivot;
		else
			parent->right = pivot;
	}
	return pivot;
}
static inline
struct rb_node * rb_rol(struct rb_node * node, struct rb_node * parent)
{
	struct rb_node * pivot = node->right;
	node->right = pivot->left;
	pivot->left = node;
	if(parent != 0)
	{
		if(parent->left == node)
			parent->left = pivot;
		else
			parent->right = pivot;
	}
	return pivot;
}
struct rb_node * rb_find(struct rb_node * self,
			 struct rb_node const * item,
			 rb_cmp cmp, int offset)
{
	while(self)
	{
		register int cmp_result = compare(item, self, cmp, offset);
		if(cmp_result > 0)
			self = self->right;
		else if(cmp_result < 0)
			self = self->left;
		else
			break;
	}
	return self;
}
struct rb_node * rb_insert(struct rb_node ** const root, struct rb_node * item,
			   rb_cmp cmp, int offset)
{
	struct rb_node * stack[64];
	register struct rb_node ** sp;
	register struct rb_node * node;
	register struct rb_node * parent;
	if(root == 0 || item == 0)
		return 0;
	if(*root == 0)
	{
		*root = rb_init_root(item);
		return item;
	}
	sp = stack - 1;
	node = *root;
	parent = 0;
	*++sp = parent;
	while(1)
	{
		register int cmp_result = compare(item, node, cmp, offset);
		if(cmp_result == 0)
			return 0;
		*++sp = node;
		if(cmp_result < 0)
			node = node->left;
		else
			node = node->right;
		if(node == 0)
		{
			node = *sp;
			if(cmp_result < 0)
				node->left = rb_init(item);
			else
				node->right = rb_init(item);
			break;
		}
	}
	register int no_fix_count = 0;
	while(sp > stack)
	{
		node = *sp--;
		parent = *sp;
		if(node->left != 0 && node->left->is_red)
		{
			no_fix_count = 0;
			if(IS_RED(node->right))
				goto split_by_recolour;
			node = rb_ror(node, parent);
			node->is_red = node->right->is_red;
			node->right->is_red = 1;
			assert(IS_BLACK(node->left));
		}
		if(IS_RED(node->right) && IS_RED(node->right->right))
		{
			no_fix_count = 0;
			node = rb_rol(node, parent);
		split_by_recolour:
			node->is_red = 1;
			node->left->is_red = 0;
			node->right->is_red = 0;
		}
		if(no_fix_count > 1)
			return item;
		no_fix_count++;
	}
	*root = rb_set_black(node);
	return item;
}
static inline
int rb_swap(struct rb_node * n1, struct rb_node * np1,
	    struct rb_node * n2, struct rb_node * np2)
{
	if(n1 == n2)
		return 0;
	struct rb_node tmp_parent = {.left = n1, .right = n2};
	if(np1 == 0)
		np1 = &tmp_parent;
	if(np2 == 0)
		np2 = &tmp_parent;
	if(n2 == np1)
	{
		struct rb_node * tmp;
		tmp = n1;
		n1 = n2;
		n2 = tmp;

		tmp = np1;
		np1 = np2;
		np2 = tmp;
	}
	if(n1 == np2)
	{

		if(np1->left == n1)
			np1->left = n2;
		else if(np1->right == n1)
			np1->right = n2;
		else
			return -1;

		struct rb_node * tmp;
		if(n1->left == n2)
		{
			n1->left = n2->left;
			n2->left = n1;

			tmp = n1->right;
			n1->right = n2->right;
			n2->right = tmp;
		}
		else if(n1->right == n2)
		{
			n1->right = n2->right;
			n2->right = n1;

			tmp = n1->left;
			n1->left = n2->left;
			n2->left = tmp;
		}
		else
			return -2;
	}
	else
	{
		struct rb_node * tmp;
		if(np1 == np2)
		{
			tmp = np1->left;
			np1->left = np1->right;
			np1->right = tmp;
		}
		else
		{
			if(np1->left == n1)
				np1->left = n2;
			else if(np1->right == n1)
				np1->right = n2;
			else
				return -3;

			if(np2->left == n2)
				np2->left = n1;
			else if(np2->right == n2)
				np2->right = n1;
			else
				return -4;
		}

		tmp = n1->left;
		n1->left = n2->left;
		n2->left = tmp;

		tmp = n1->right;
		n1->right = n2->right;
		n2->right = tmp;
	}
	return 0;
}
struct rb_node * rb_delete(struct rb_node ** const root, struct rb_node * item,
			   rb_cmp cmp, int offset)
{
	struct rb_node * stack[64];
	struct rb_node ** sp = stack - 1;
	struct rb_node ** targ_p = 0;
	struct rb_node ** targ = 0;

	struct rb_node * parent = 0;
	struct rb_node * node = *root;
	int cmp_result;
	*++sp = 0;
	while(node != 0)
	{
		*++sp = node;
		if(targ == 0)
		{
			cmp_result = compare(item, node, cmp, offset);
			if(cmp_result == 0)
			{
				targ = sp;
				targ_p = sp - 1;
				item = node;
			}
			if(cmp_result > 0)
				node = node->right;
			else
				node = node->left;
		}
		else
		{
			node = node->right;
		}
	}
	/* return null if target is not found*/
	if(targ == 0)
		return 0;


	struct rb_node ** const leaf = sp;
	struct rb_node ** const leaf_p = sp - 1;
	/* swap target and leaf*/

	if(leaf != targ)
	{
		rb_swap(*leaf, *leaf_p, *targ, *targ_p);
		{
			unsigned long tmp = (*targ)->is_red;
			(*targ)->is_red = (*leaf)->is_red;
			(*leaf)->is_red = tmp;
		}
		{
			struct rb_node * tmp = *targ;
			*targ = *leaf;
			*leaf = tmp;
		}
		*root =stack[1];
	}

	/* delete leaf root*/
	if(*leaf == *root)
	{
		assert(*leaf_p == 0);
		*root = rb_set_black((*root)->right);
		return item;
	}

	/* delete red leaf*/
	if(IS_RED(*leaf))
	{
		(*leaf_p)->right = 0;
		return item;
	}

	/* delete 3-leaf*/
	if(IS_3(*leaf))
	{
		if(*leaf == (*leaf_p)->left)
			(*leaf_p)->left = (*leaf)->right;
		else
			(*leaf_p)->right = (*leaf)->right;
		rb_set_black((*leaf)->right);
		return item;
	}

	/* remove leaf from its parent*/
	if(*leaf == (*leaf_p)->left)
		(*leaf_p)->left = 0;
	else if(*leaf == (*leaf_p)->right)
		(*leaf_p)->right = 0;
	struct rb_node * dangle = 0;
	sp = leaf_p;
	while(1)
	{
		node = *sp--;
		parent = *sp;
		if(IS_BLACK(node))
		{
			if(IS_3(node) || IS_3(node->right) || IS_3(node->left))
				break;
			if(node->left == 0)
			{
				node->left = dangle;
				node->right->is_red = 1;
			}
			else if(node->right == 0)
			{
				assert(node->right == 0);
				node->right = dangle;
				node = rb_ror(node, parent);
				node->right->is_red = 1;
			}
			dangle = node;
			if(parent != 0)
			{
				if(parent->left == node)
					parent->left = 0;
				else if(parent->right == node)
					parent->right = 0;
			}
		}
		if(sp <= stack)
		{

			*root = rb_set_black(node);
			return item;
		}
	}

	if(IS_2(node))
	{

		if(node->left == 0)
		{
			assert(IS_3(node->right));

			node->left = dangle;
			node = rb_rol(node, parent);

			node->right->is_red = 0;
		}
		else if(node->right == 0)
		{
			assert(IS_3(node->left));

			node->right = dangle;
			rb_rol(node->left, node);
			node = rb_ror(node, parent);

			node->is_red = 0;
		}
	}
	else
	{
		assert(IS_3(node));
		if(node->left == 0)
		{
			node->left = dangle;
			if(IS_3(node->right->left))
			{
				rb_ror(node->right, node);
				node = rb_rol(node, parent);

				node->right->left->is_red = 0;
			}
			else if(IS_3(node->right->right))
			{
				rb_ror(node->right, node);
				node = rb_rol(node, parent);
				rb_rol(node->right, node);

				node->right->is_red = 1;
				node->right->left->is_red = 0;
				node->right->right->is_red = 0;
			}
			else
			{
				assert(IS_2(node->right->left));
				assert(IS_2(node->right->right));

				node = rb_rol(node, parent);

				node->is_red = 0;
				node->left->right->is_red = 1;
			}
		}
		else if(node->right->left == 0)
		{
			node->right->left = dangle;
			if(IS_3(node->right->right))
			{
				rb_rol(node->right, node);

				node->right->is_red = 1;
				node->right->left->is_red = 0;
				node->right->right->is_red = 0;
			}
			else if(IS_3(node->left))
			{
				rb_rol(node->left, node);
				node = rb_ror(node, parent);
				rb_rol(node->right, node);

				node->is_red = 0;
			}
			else
			{
				assert(IS_2(node->left));
				assert(IS_2(node->right->right));

				node->right->is_red = 0;
				node->right->right->is_red = 1;
			}
		}
		else if(node->right->right == 0)
		{
			node->right->right = dangle;
			if(IS_3(node->right->left))
			{
				rb_rol(node->right->left, node->right);
				rb_ror(node->right, node);

				node->right->is_red = 1;
				node->right->left->is_red = 0;
				node->right->right->is_red = 0;
			}
			else if(IS_3(node->left))
			{
				rb_rol(node->left, node);
				rb_ror(node->right, node);
				node = rb_ror(node, parent);
				rb_rol(node->right, node);

				node->is_red = 0;
				node->right->is_red = 1;
				node->right->right->is_red = 0;
			}
			else
			{
				assert(IS_2(node->left));
				assert(IS_2(node->right->left));

				rb_ror(node->right, node);
			}
		}

	}

	if(parent == 0)
		*root = rb_set_black(node);
	return item;
}
