

/*-
 * Copyright 1997-1999, 2001, John-Mark Gurney.
 *           2008, Attractive Chaos <attractivechaos@aol.co.uk>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __AC_KBTREE_H
#define __AC_KBTREE_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* header of a node. a node is like this:
|==kbnode_t=|=========key_t[n]==========|=========kbnode_t[n + 1]=========|
|<-4 bytes->|<-n * sizeof(key_t) bytes->|<-(n + 1) * sizeof(void*) bytes->|

*/
typedef struct {
    int32_t is_internal:1, n:31;
} kbnode_t;

#define KB_KEYS(type, x)   ((type*)((char*)x + 4))
#define KB_NODES(btr, x)    ((kbnode_t**)((char*)x + btr->off_ptr))

#define __KB_TREE_T(name)                       \
    typedef struct {                            \
        kbnode_t *root;                         \
        int off_ptr;/*off_ptr is the offset of the child node array*/\
	int ilen;/*ilen is the size of an internal node. it is a multiple of four.*/\
	int elen;/*elen is the size of a leaf node it is a multiple of four.*/\
        int n;/* n is the number of keys in one node*/\
	int t;/* t is the minimum number of child nodes*/\
        int n_keys, n_nodes;                    \
    } kbtree_##name##_t;

#define __KB_INIT(name, key_t)                                          \
    kbtree_##name##_t *kb_init_##name(int size)                         \
    {                                                                   \
        kbtree_##name##_t *b;                                           \
        b = (kbtree_##name##_t*)calloc(1, sizeof(kbtree_##name##_t));   \
        b->t = (size + 3) / 4 * 4 / 2; \
        if (b->t < 2) {                                                 \
            free(b); return 0;                                          \
        }                                                               \
        b->n = 2 * b->t - 1;                                            \
	b->off_ptr = 4 + b->n * sizeof(key_t); 				\
        b->ilen = (4 + sizeof(void*) + b->n * (sizeof(void*) + sizeof(key_t)) + 3) / 4 * 4; \
        b->elen = (b->off_ptr + 3) / 4 * 4;                             \
        b->root = (kbnode_t*)calloc(1, b->ilen);                        \
        ++b->n_nodes;                                                   \
        return b;                                                       \
    }

#define __kb_destroy(b) do {                                            \
        int i, max = 8;                                                 \
        kbnode_t *x, **top, **stack;                                    \
        if (b) {                                                        \
            top = stack = (kbnode_t**)calloc(max, sizeof(kbnode_t*));   \
            *top++ = (b)->root;                                         \
            while (top != stack) {                                      \
                x = *--top;                                             \
                if (x->is_internal == 0) { free(x); continue; }         \
                for (i = 0; i <= x->n; ++i)                             \
                    if (KB_NODES(b, x)[i]) {                            \
                        if (top - stack == max) {                       \
                            max <<= 1;                                  \
                            stack = (kbnode_t**)realloc(stack, max * sizeof(kbnode_t*)); \
                            top = stack + (max>>1);                     \
                        }                                               \
                        *top++ = KB_NODES(b, x)[i];                     \
                    }                                                   \
                free(x);                                                \
            }                                                           \
        }                                                               \
        free(b); free(stack);                                           \
    } while (0)

#define __KB_GET_AUX0(name, key_t, __cmp)                               \
    static inline int __kb_get_aux_##name(const kbnode_t * __restrict x, const key_t * __restrict k, int *r) \
    {                                                                   \
        int tr, *rr, begin, end, n = x->n / 2;                          \
        if (x->n == 0) return -1;                                       \
        if (__cmp(*k, KB_KEYS(key_t, x)[n]) < 0) {                     \
            begin = 0; end = n;                                         \
        } else { begin = n; end = x->n - 1; }                           \
        rr = r? r : &tr;                                                \
        n = end;                                                        \
        while (n >= begin && (*rr = __cmp(*k, KB_KEYS(key_t, x)[n])) < 0) --n; \
        return n;                                                       \
    }

#define __KB_GET_AUX1(name, key_t, __cmp)                               \
    static inline int __kb_getp_aux_##name(const kbnode_t * __restrict x, const key_t * __restrict k, int *r) \
    {                                                                   \
        int tr, *rr, begin = 0, end = x->n;                             \
        if (x->n == 0) return -1;                                       \
        rr = r? r : &tr;                                                \
        while (begin < end) {                                           \
            int mid = (begin + end) / 2;                                \
            if (__cmp(KB_KEYS(key_t, x)[mid], *k) < 0) begin = mid + 1; \
            else end = mid;                                             \
        }                                                               \
        if (begin == x->n) { *rr = 1; return x->n - 1; }                \
        if ((*rr = __cmp(*k, KB_KEYS(key_t, x)[begin])) < 0) --begin;  \
        return begin;                                                   \
    }

#define __KB_GET(name, key_t)                                           \
    static key_t *kb_getp_##name(kbtree_##name##_t *b, const key_t * __restrict k) \
    {                                                                   \
        int i, r = 0;                                                   \
        kbnode_t *x = b->root;                                          \
        while (x) {                                                     \
            i = __kb_getp_aux_##name(x, k, &r);                         \
            if (i >= 0 && r == 0) return &KB_KEYS(key_t, x)[i];        \
            if (x->is_internal == 0) return 0;                          \
            x = KB_NODES(b, x)[i + 1];                                  \
        }                                                               \
        return 0;                                                       \
    }                                                                   \
    static inline key_t *kb_get_##name(kbtree_##name##_t *b, const key_t k) \
    {                                                                   \
        return kb_getp_##name(b, &k);                                   \
    }

#define __KB_INTERVAL(name, key_t)                                      \
    static void kb_intervalp_##name(kbtree_##name##_t *b, const key_t * __restrict k, key_t **lower, key_t **upper) \
    {                                                                   \
        int i, r = 0;                                                   \
        kbnode_t *x = b->root;                                          \
        *lower = *upper = 0;                                            \
        while (x) {                                                     \
            i = __kb_getp_aux_##name(x, k, &r);                         \
            if (i >= 0 && r == 0) {                                     \
                *lower = *upper = &KB_KEYS(key_t, x)[i];               \
                return;                                                 \
            }                                                           \
            if (i >= 0) *lower = &KB_KEYS(key_t, x)[i];                \
            if (i < x->n - 1) *upper = &KB_KEYS(key_t, x)[i + 1];      \
            if (x->is_internal == 0) return;                            \
            x = KB_NODES(b, x)[i + 1];                                  \
        }                                                               \
    }                                                                   \
    static inline void kb_interval_##name(kbtree_##name##_t *b, const key_t k, key_t **lower, key_t **upper) \
    {                                                                   \
        kb_intervalp_##name(b, &k, lower, upper);                       \
    }

#define __KB_PUT(name, key_t, __cmp)                                    \
    /* x must be an internal node */                                    \
    static void __kb_split_##name(kbtree_##name##_t *b, kbnode_t *x, int i, kbnode_t *y) \
    {                                                                   \
        kbnode_t *z = calloc(1, y->is_internal? b->ilen : b->elen);    \
        ++b->n_nodes;                                                   \
        z->is_internal = y->is_internal;                                \
        z->n = b->t - 1;                                                \
        memcpy(KB_KEYS(key_t, z), KB_KEYS(key_t, y) + b->t, sizeof(key_t) * (b->t - 1)); \
        if (y->is_internal) memcpy(KB_NODES(b, z), KB_NODES(b, y) + b->t, sizeof(void*) * b->t); \
        y->n = b->t - 1;                                                \
        memmove(KB_NODES(b, x) + i + 2, KB_NODES(b, x) + i + 1, sizeof(void*) * (x->n - i)); \
        KB_NODES(b, x)[i + 1] = z;                                      \
        memmove(KB_KEYS(key_t, x) + i + 1, KB_KEYS(key_t, x) + i, sizeof(key_t) * (x->n - i)); \
        KB_KEYS(key_t, x)[i] = KB_KEYS(key_t, y)[b->t - 1];           \
        ++x->n;                                                         \
    }                                                                   \
    static void __kb_putp_aux_##name(kbtree_##name##_t *b, kbnode_t *x, const key_t * __restrict k) \
    {                                                                   \
        if (x->is_internal != 0) {                                      \
            int i = __kb_getp_aux_##name(x, k, 0) + 1;                      \
            if (KB_NODES(b, x)[i]->n == b->n) {                 \
                __kb_split_##name(b, x, i, KB_NODES(b, x)[i]);          \
                if (__cmp(*k, KB_KEYS(key_t, x)[i]) > 0) ++i;          \
            }                                                           \
            __kb_putp_aux_##name(b, KB_NODES(b, x)[i], k);              \
        } else {                                                        \
            int i = __kb_getp_aux_##name(x, k, 0);                          \
            if (i != x->n - 1)                                          \
                memmove(KB_KEYS(key_t, x) + i + 2, KB_KEYS(key_t, x) + i + 1, (x->n - i - 1) * sizeof(key_t)); \
            KB_KEYS(key_t, x)[i + 1] = *k;                             \
            ++x->n;                                                     \
        }                                                               \
    }                                                                   \
    static void kb_putp_##name(kbtree_##name##_t *b, const key_t * __restrict k) \
    {                                                                   \
        kbnode_t *r;/* root*/\
	kbnode_t *s;                                                \
        ++b->n_keys;                                                    \
        r = b->root;                                                    \
        if (r->n == b->n) {   /*if root is a full node*/                                  \
            ++b->n_nodes;                                               \
            s = (kbnode_t*)calloc(1, b->ilen);/* make a new root*/                          \
            b->root = s; s->is_internal = 1; s->n = 0;   /* assign new root*/               \
            KB_NODES(b, s)[0] = r;                                      \
            __kb_split_##name(b, s, 0, r); /*split it*/                             \
            r = s;                                                    \
        }                                                               \
        __kb_putp_aux_##name(b, r, k);                                  \
    }                                                                   \
    static inline void kb_put_##name(kbtree_##name##_t *b, const key_t k) \
    {                                                                   \
        kb_putp_##name(b, &k);                                          \
    }


#define __KB_DEL(name, key_t)                                           \
    static key_t __kb_delp_aux_##name(kbtree_##name##_t *b, kbnode_t *x, const key_t * __restrict k, int s) \
    {                                                                   \
        int yn, zn, i, r = 0;                                           \
        kbnode_t *xp, *y, *z;                                           \
        key_t kp;                                                       \
        if (x == 0) return *k;                                          \
        if (s) { /* s can only be 0, 1 or 2 */                          \
            r = x->is_internal == 0? 0 : s == 1? 1 : -1;                \
            i = s == 1? x->n - 1 : -1;                                  \
        } else i = __kb_getp_aux_##name(x, k, &r);                      \
        if (x->is_internal == 0) {                                      \
            if (s == 2) ++i;                                            \
            kp = KB_KEYS(key_t, x)[i];                                 \
            memmove(KB_KEYS(key_t, x) + i, KB_KEYS(key_t, x) + i + 1, (x->n - i - 1) * sizeof(key_t)); \
            --x->n;                                                     \
            return kp;                                                  \
        }                                                               \
        if (r == 0) {                                                   \
            if ((yn = KB_NODES(b, x)[i]->n) >= b->t) {                  \
                xp = KB_NODES(b, x)[i];                                 \
                kp = KB_KEYS(key_t, x)[i];                             \
                KB_KEYS(key_t, x)[i] = __kb_delp_aux_##name(b, xp, 0, 1); \
                return kp;                                              \
            } else if ((zn = KB_NODES(b, x)[i + 1]->n) >= b->t) {       \
                xp = KB_NODES(b, x)[i + 1];                             \
                kp = KB_KEYS(key_t, x)[i];                             \
                KB_KEYS(key_t, x)[i] = __kb_delp_aux_##name(b, xp, 0, 2); \
                return kp;                                              \
            } else if (yn == b->t - 1 && zn == b->t - 1) {              \
                y = KB_NODES(b, x)[i]; z = KB_NODES(b, x)[i + 1];       \
                KB_KEYS(key_t, y)[y->n++] = *k;                        \
                memmove(KB_KEYS(key_t, y) + y->n, KB_KEYS(key_t, z), z->n * sizeof(key_t)); \
                if (y->is_internal) memmove(KB_NODES(b, y) + y->n, KB_NODES(b, z), (z->n + 1) * sizeof(void*)); \
                y->n += z->n;                                           \
                memmove(KB_KEYS(key_t, x) + i, KB_KEYS(key_t, x) + i + 1, (x->n - i - 1) * sizeof(key_t)); \
                memmove(KB_NODES(b, x) + i + 1, KB_NODES(b, x) + i + 2, (x->n - i - 1) * sizeof(void*)); \
                --x->n;                                                 \
                free(z);                                                \
                return __kb_delp_aux_##name(b, y, k, s);                \
            }                                                           \
        }                                                               \
        ++i;                                                            \
        if ((xp = KB_NODES(b, x)[i])->n == b->t - 1) {                  \
            if (i > 0 && (y = KB_NODES(b, x)[i - 1])->n >= b->t) {      \
                memmove(KB_KEYS(key_t, xp) + 1, KB_KEYS(key_t, xp), xp->n * sizeof(key_t)); \
                if (xp->is_internal) memmove(KB_NODES(b, xp) + 1, KB_NODES(b, xp), (xp->n + 1) * sizeof(void*)); \
                KB_KEYS(key_t, xp)[0] = KB_KEYS(key_t, x)[i - 1];     \
                KB_KEYS(key_t, x)[i - 1] = KB_KEYS(key_t, y)[y->n - 1]; \
                if (xp->is_internal) KB_NODES(b, xp)[0] = KB_NODES(b, y)[y->n]; \
                --y->n; ++xp->n;                                        \
            } else if (i < x->n && (y = KB_NODES(b, x)[i + 1])->n >= b->t) { \
                KB_KEYS(key_t, xp)[xp->n++] = KB_KEYS(key_t, x)[i];   \
                KB_KEYS(key_t, x)[i] = KB_KEYS(key_t, y)[0];          \
                if (xp->is_internal) KB_NODES(b, xp)[xp->n] = KB_NODES(b, y)[0]; \
                --y->n;                                                 \
                memmove(KB_KEYS(key_t, y), KB_KEYS(key_t, y) + 1, y->n * sizeof(key_t)); \
                if (y->is_internal) memmove(KB_NODES(b, y), KB_NODES(b, y) + 1, (y->n + 1) * sizeof(void*)); \
            } else if (i > 0 && (y = KB_NODES(b, x)[i - 1])->n == b->t - 1) { \
                KB_KEYS(key_t, y)[y->n++] = KB_KEYS(key_t, x)[i - 1]; \
                memmove(KB_KEYS(key_t, y) + y->n, KB_KEYS(key_t, xp), xp->n * sizeof(key_t)); \
                if (y->is_internal) memmove(KB_NODES(b, y) + y->n, KB_NODES(b, xp), (xp->n + 1) * sizeof(void*)); \
                y->n += xp->n;                                          \
                memmove(KB_KEYS(key_t, x) + i - 1, KB_KEYS(key_t, x) + i, (x->n - i) * sizeof(key_t)); \
                memmove(KB_NODES(b, x) + i, KB_NODES(b, x) + i + 1, (x->n - i) * sizeof(void*)); \
                --x->n;                                                 \
                free(xp);                                               \
                xp = y;                                                 \
            } else if (i < x->n && (y = KB_NODES(b, x)[i + 1])->n == b->t - 1) { \
                KB_KEYS(key_t, xp)[xp->n++] = KB_KEYS(key_t, x)[i];   \
                memmove(KB_KEYS(key_t, xp) + xp->n, KB_KEYS(key_t, y), y->n * sizeof(key_t)); \
                if (xp->is_internal) memmove(KB_NODES(b, xp) + xp->n, KB_NODES(b, y), (y->n + 1) * sizeof(void*)); \
                xp->n += y->n;                                          \
                memmove(KB_KEYS(key_t, x) + i, KB_KEYS(key_t, x) + i + 1, (x->n - i - 1) * sizeof(key_t)); \
                memmove(KB_NODES(b, x) + i + 1, KB_NODES(b, x) + i + 2, (x->n - i - 1) * sizeof(void*)); \
                --x->n;                                                 \
                free(y);                                                \
            }                                                           \
        }                                                               \
        return __kb_delp_aux_##name(b, xp, k, s);                       \
    }                                                                   \
    static key_t kb_delp_##name(kbtree_##name##_t *b, const key_t * __restrict k) \
    {                                                                   \
        kbnode_t *x;                                                    \
        key_t ret;                                                      \
        ret = __kb_delp_aux_##name(b, b->root, k, 0);                   \
        --b->n_keys;                                                    \
        if (b->root->n == 0 && b->root->is_internal) {                  \
            --b->n_nodes;                                               \
            x = b->root;                                                \
            b->root = KB_NODES(b, x)[0];                                \
            free(x);                                                    \
        }                                                               \
        return ret;                                                     \
    }                                                                   \
    static inline key_t kb_del_##name(kbtree_##name##_t *b, const key_t k) \
    {                                                                   \
        return kb_delp_##name(b, &k);                                   \
    }

typedef struct {
    kbnode_t *x;
    int i;
} __kbstack_t;

#define __kb_traverse(key_t, b, __func) do {                            \
        int __kmax = 8;                                                 \
        __kbstack_t *__kstack, *__kp;                                   \
        __kp = __kstack = (__kbstack_t*)calloc(__kmax, sizeof(__kbstack_t)); \
        __kp->x = (b)->root; __kp->i = 0;                               \
        for (;;) {                                                      \
            while (__kp->x && __kp->i <= __kp->x->n) {                  \
                if (__kp - __kstack == __kmax - 1) {                    \
                    __kmax <<= 1;                                       \
                    __kstack = (__kbstack_t*)realloc(__kstack, __kmax * sizeof(__kbstack_t)); \
                    __kp = __kstack + (__kmax>>1) - 1;                  \
                }                                                       \
                (__kp+1)->i = 0; (__kp+1)->x = __kp->x->is_internal? KB_NODES(b, __kp->x)[__kp->i] : 0; \
                ++__kp;                                                 \
            }                                                           \
            --__kp;                                                     \
            if (__kp >= __kstack) {                                     \
                if (__kp->x && __kp->i < __kp->x->n) __func(&KB_KEYS(key_t, __kp->x)[__kp->i]); \
                ++__kp->i;                                              \
            } else break;                                               \
        }                                                               \
        free(__kstack);                                                 \
    } while (0)

#define KBTREE_INIT(name, key_t, __cmp)         \
    __KB_TREE_T(name)                           \
    __KB_INIT(name, key_t)                      \
    __KB_GET_AUX1(name, key_t, __cmp)           \
    __KB_GET(name, key_t)                       \
    __KB_INTERVAL(name, key_t)                  \
    __KB_PUT(name, key_t, __cmp)                \
    __KB_DEL(name, key_t)

#define KB_DEFAULT_SIZE 512

#define kbtree_t(name) kbtree_##name##_t
#define kb_init(name, s) kb_init_##name(s)
#define kb_destroy(name, b) __kb_destroy(b)
#define kb_get(name, b, k) kb_get_##name(b, k)
#define kb_put(name, b, k) kb_put_##name(b, k)
#define kb_del(name, b, k) kb_del_##name(b, k)
#define kb_interval(name, b, k, l, u) kb_interval_##name(b, k, l, u)
#define kb_getp(name, b, k) kb_getp_##name(b, k)
#define kb_putp(name, b, k) kb_putp_##name(b, k)
#define kb_delp(name, b, k) kb_delp_##name(b, k)
#define kb_intervalp(name, b, k, l, u) kb_intervalp_##name(b, k, l, u)

#define kb_size(b) ((b)->n_keys)

#define kb_generic_cmp(a, b) (((a) > (b)) - ((a) < (b)))
#define kb_str_cmp(a, b) strcmp(a, b)

#endif


