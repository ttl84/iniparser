#ifndef RB_H
#define RB_H
#define RB_ISB(nd)\
(!(nd) || !(nd)->is_red)
#define RB_ISR(nd)\
((nd) && (nd)->is_red)
#define RB_MAKE(name, type, cmp)\
struct rbnode##name{\
	type item;\
	struct rbnode##name * l;\
	struct rbnode##name * r;\
	unsigned is_red;\
};\
typedef struct rbnode##name rbnode##name;\
struct name{\
	struct rbnode##name * root;\
};\
static void name##_init(struct name * t)\
{\
	t->root = NULL;\
}\
static rbnode##name * rb##name##mknde(type item)\
{\
	rbnode##name * nde = calloc(1, sizeof *nde);\
	if(nde)\
	{\
		nde->item = item;\
		nde->is_red = 1;\
	}\
	return nde;\
}\
static rbnode##name* rb##name##ror(rbnode##name * node, rbnode##name * parent)\
{\
	rbnode##name * pivot = node->l;\
	node->l = pivot->r;\
	pivot->r = node;\
	if(parent)\
	{\
		if(parent->l == node)\
			parent->l = pivot;\
		else\
			parent->r = pivot;\
	}\
	return pivot;\
}\
static rbnode##name * rb##name##rol(rbnode##name * node, rbnode##name * parent)\
{\
	rbnode##name * pivot = node->r;\
	node->r = pivot->l;\
	pivot->l = node;\
	if(parent != 0)\
	{\
		if(parent->l == node)\
			parent->l = pivot;\
		else\
			parent->r = pivot;\
	}\
	return pivot;\
}\
static type * name##_find(struct name * rb, type item)\
{\
	rbnode##name * self = rb->root;\
	while(self)\
	{\
		int result = cmp(item, self->item);\
		if(result > 0)\
			self = self->r;\
		else if(result < 0)\
			self = self->l;\
		else\
			return &self->item;\
	}\
	return NULL;\
}\
static int rb##name##insert_rec(rbnode##name * self,\
	rbnode##name * p0,\
	rbnode##name * p1,\
	rbnode##name * p2,\
	type item)\
{\
	if(RB_ISB(self) && RB_ISR(self->l) && RB_ISR(self->r))\
	{\
		self->is_red = 1;\
		self->l->is_red = 0;\
		self->r->is_red = 0;\
		if(p0->is_red)\
		{\
			if(self == p0->l && p0 == p1->l)\
			{\
				rb##name##ror(p1, p2);\
				p0->is_red = 0;\
				p1->is_red = 1;\
				p1 = p2;\
				p2 = NULL;\
			}\
			else if(self == p0->r && p0 == p1->r)\
			{\
				rb##name##rol(p1, p2);\
				p0->is_red = 0;\
				p1->is_red = 1;\
				p1 = p2;\
				p2 = NULL;\
			}\
			else if(self == p0->l && p0 == p1->r)\
			{\
				rb##name##ror(p0, p1);\
				rb##name##rol(p1, p2);\
				self->is_red = 0;\
				p1->is_red = 1;\
				p0 = p2;\
				p1 = NULL;\
				p2 = NULL;\
			}\
			else if(self == p0->r && p0 == p1->l)\
			{\
				rb##name##rol(p0, p1);\
				rb##name##ror(p1, p2);\
				self->is_red = 0;\
				p1->is_red = 1;\
				p0 = p2;\
				p1 = NULL;\
				p2 = NULL;\
			}\
		}\
	}\
	int result = cmp(item, self->item);\
	if(result > 0)\
	{\
		if(self->r)\
			return rb##name##insert_rec(self->r, self, p0, p1, item);\
		else\
		{\
			self->r = rb##name##mknde(item);\
			if(self->is_red)\
			{\
				if(self == p0->r)\
				{\
					rb##name##rol(p0, p1);\
					self->is_red = 0;\
					p0->is_red = 1;\
				}\
				else\
				{\
					self = rb##name##rol(self, p0);\
					rb##name##ror(p0, p1);\
					self->is_red = 0;\
					p0->is_red = 1;\
				}\
			}\
			return 0;\
		}\
	}\
	else if(result < 0)\
	{\
		if(self->l)\
			return rb##name##insert_rec(self->l, self, p0, p1, item);\
		else\
		{\
			self->l = rb##name##mknde(item);\
			if(self->is_red)\
			{\
				if(self == p0->l)\
				{\
					rb##name##ror(p0, p1);\
					self->is_red = 0;\
					p0->is_red = 1;\
				}\
				else\
				{\
					self = rb##name##ror(self, p0);\
					rb##name##rol(p0, p1);\
					self->is_red = 0;\
					p0->is_red = 1;\
				}\
			}\
			return 0;\
		}\
	}\
	else\
	{\
		return 1;\
	}\
}\
static int name##_insert(struct name * rb, type item)\
{\
	if(rb->root)\
	{\
		rbnode##name	p0 = {.l = rb->root, .r = NULL, .is_red = 0},\
				p1 = {.l = &p0, .r = NULL, .is_red = 0},\
				p2 = {.l = &p1, .r = NULL, .is_red = 0};\
		int result = rb##name##insert_rec(rb->root, &p0, &p1, &p2, item);\
		rb->root = p0.l;\
		rb->root->is_red = 0;\
		return result;\
	}\
	else\
	{\
		rb->root = rb##name##mknde(item);\
		rb->root->is_red = 0;\
		return 0;\
	}\
}\
static int rb##name##remove_rec(rbnode##name * self, rbnode##name * p0,\
	rbnode##name * target, type item)\
{\
	int go_right;\
	if(target)\
		go_right = 1;\
	else\
	{\
		int result = cmp(item, self->item);\
		go_right = (result > 0);\
		if(result == 0)\
			target = self;\
	}\
	if(go_right)\
	{\
		if(RB_ISB(self->r) && RB_ISR(self->l))\
		{\
			p0 = rb##name##ror(self, p0);\
			/*p1 = p0;*/\
			p0->is_red = 0;\
			self->is_red = 1;\
		}\
	}\
	else\
	{\
		if(RB_ISB(self->l) && RB_ISR(self->r))\
		{\
			p0 = rb##name##rol(self, p0);\
			/*p1 = p0;*/\
			p0->is_red = 0;\
			self->is_red = 1;\
		}\
	}\
	if(self->l && !self->l->is_red && self->r && !self->r->is_red)\
	{\
		if(	self->is_red &&\
			RB_ISB(self->l->l) && RB_ISB(self->l->r) &&\
			RB_ISB(self->r->l) && RB_ISB(self->r->r))\
		{\
			self->is_red = 0;\
			self->l->is_red = 1;\
			self->r->is_red = 1;\
		}\
		else if(go_right)\
		{\
			if(RB_ISB(self->r->l) && RB_ISB(self->r->r))\
			{\
				if(RB_ISR(self->l->r))\
				{\
					rb##name##rol(self->l, self);\
					p0 = rb##name##ror(self, p0);\
					p0->is_red = self->is_red;\
					self->is_red = 0;\
					self->r->is_red = 1;\
				}\
				else if(RB_ISR(self->l->l))\
				{\
					self->l->l->is_red = 0;\
					p0 = rb##name##ror(self, p0);\
					p0->is_red = self->is_red;\
					self->is_red = 0;\
					self->r->is_red = 1;\
				}\
			}\
		}\
		else\
		{\
			if(RB_ISB(self->l->r) && RB_ISB(self->l->l))\
			{\
				if(RB_ISR(self->r->l))\
				{\
					rb##name##ror(self->r, self);\
					p0 = rb##name##rol(self, p0);\
					p0->is_red = self->is_red;\
					self->is_red = 0;\
					self->l->is_red = 1;\
				}\
				else if(RB_ISR(self->r->r))\
				{\
					self->r->r->is_red = 0;\
					p0 = rb##name##rol(self, p0);\
					p0->is_red = self->is_red;\
					self->is_red = 0;\
					self->l->is_red = 1;\
				}\
			}\
		}\
	}\
	rbnode##name * next;\
	if(go_right)\
	{\
		if(self->r)\
			next = self->r;\
		else if(target)\
		{\
			target->item = self->item;\
			if(self->l)\
			{\
				p0 = rb##name##ror(self, p0);\
				p0->is_red = 0;\
			}\
			if(self == p0->l)\
				p0->l = NULL;\
			else\
				p0->r = NULL;\
			free(self);\
			return 0;\
		}\
		else\
		{\
			return 1;\
		}\
	}\
	else\
	{\
		if(self->l)\
			next = self->l;\
		else if(target)\
		{\
			target->item = self->item;\
			if(self->r)\
			{\
				p0 = rb##name##rol(self, p0);\
				p0->is_red = 0;\
			}\
			if(self == p0->l)\
				p0->l = NULL;\
			else\
				p0->r = NULL;\
			free(self);\
			return 0;\
		}\
		else\
		{\
			return 1;\
		}\
	}\
	return rb##name##remove_rec(next, self, target, item);\
}\
static int name##_remove(struct name * rb, type item)\
{\
	if(rb->root)\
	{\
		rbnode##name parent = {.l = rb->root, .is_red = 0};\
		if(RB_ISB(rb->root->l) && RB_ISB(rb->root->r))\
			rb->root->is_red = 1;\
		int result = rb##name##remove_rec(rb->root, &parent, NULL, item);\
		rb->root = parent.l;\
		if(rb->root)\
			rb->root->is_red = 0;\
		return result;\
	}\
	else\
	{\
		return -1;\
	}\
}\
static int rbnode##name##lheight(rbnode##name * nd)\
{\
	return nd ? (!nd->is_red) + rbnode##name##lheight(nd->l) : 1;\
}\
static int rbnode##name##rheight(rbnode##name * nd)\
{\
	return nd ? (!nd->is_red) + rbnode##name##rheight(nd->r) : 1;\
}\
static int rbnode##name##saneheight(rbnode##name * nd)\
{\
	if(nd)\
	{\
		int lh = rbnode##name##lheight(nd);\
		int rh = rbnode##name##rheight(nd);\
		if(lh == rh)\
		{\
			return rbnode##name##saneheight(nd->l) &&\
				rbnode##name##saneheight(nd->r);\
		}\
		else\
			return 0;\
	}\
	else\
		return 1;\
}\
static int rbnode##name##sane(rbnode##name * nd)\
{\
	if(nd)\
	{\
		if(!(RB_ISR(nd) && (RB_ISR(nd->l) || RB_ISR(nd->r))))\
			return rbnode##name##sane(nd->l) &&\
				rbnode##name##sane(nd->r);\
		else\
			return 0;\
	}\
	else\
		return 1;\
}\
static int name##_sane(struct name * rb)\
{\
	return rbnode##name##saneheight(rb->root) &&\
		rbnode##name##sane(rb->root);\
}
#endif
