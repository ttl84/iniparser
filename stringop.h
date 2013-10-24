#ifndef STRINGOP_H
#define STRINGOP_H
#include <stdbool.h>
/* removes trailing whitespaces*/
void cstr_chomp(char * cstr);

bool cstr_has(char const * cstr, char c);

/* string substitution.
all occurrences of a is changed to b.
make sure cstr has enough space.*/
void cstr_subst(char * restrict cstr, char const * restrict a, char const * restrict b);

/* converts a string to an int.
intptr: the int pointed to by this pointer will be set to the converted value.

returns:
- zero for success
- non-zero for failure*/
int cstr2int(char const *, int * intptr);
int cstr2uint(char const *, unsigned * uintptr);
double cstr2double(char const *, char const ** endptr);

/* make a new copy of a char array string*/
char * cstr_dup(char const *);

/* checks if a char is whitespace*/
int is_whitespace(char c);

/* checks if a string is represents a valid int*/
int is_int(char const * cstr);
#endif // STRINGOP_H
