#ifndef VARSTR_H
#define VARSTR_H
struct varstr;
struct varstr * varstr_new(void);
void varstr_del(struct varstr *);
unsigned long varstr_len(struct varstr const*);
int varstr_append(struct varstr *, char);
int varstr_get(struct varstr *, unsigned long i);
int varstr_set(struct varstr *, unsigned long i, char c);
char const * varstr_view(struct varstr*);
void varstr_clear(struct varstr*);
#endif
