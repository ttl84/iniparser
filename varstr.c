#include "varstr.h"
#include <stdlib.h>
#include <string.h>
struct varstr{
	unsigned long buflen, len;
	char * buf;
};
struct varstr * varstr_new(void)
{
	struct varstr * newstr = calloc(1, sizeof *newstr);
	if(newstr != NULL)
	{
		char * buf = calloc(1, 1);
		if(buf != NULL)
		{
			newstr->buflen = 1;
			newstr->buf = buf;
		}
		else
		{
			free(newstr);
			newstr = NULL;
		}
	}
	return newstr;
}
void varstr_del(struct varstr * str)
{
	if(str != NULL)
	{
		free(str->buf);
		free(str);
	}
}
unsigned long varstr_len(struct varstr const * str)
{
	return str->len;
}
int varstr_append(struct varstr * str, char c)
{
	if(str->len + 1 == str->buflen)
	{
		char * newbuf = realloc(str->buf, str->buflen * 2);
		if(newbuf != NULL)
		{
			str->buf = newbuf;
			str->buflen *= 2;
		}
		else
			return -1;
	}
	str->buf[str->len] = c;
	str->len++;
	str->buf[str->len] = 0;
	return 0;
}
int varstr_get(struct varstr * str, unsigned long i)
{
	if(i < str->len)
		return str->buf[i];
	else
		return -1;
}
int varstr_set(struct varstr * str, unsigned long i, char c)
{
	if(i < str->len)
	{
		str->buf[i] = c;
		return 0;
	}
	else
		return -1;
}
char const * varstr_view(struct varstr * str)
{
	return str->buf;
}
void varstr_clear(struct varstr * str)
{
	str->len = 0;
	str->buf[0] = 0;
}
