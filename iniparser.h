#ifndef INIPARSER_H
#define INIPARSER_H
#include <stdio.h>
struct ini;

struct ini * ini_new(void);
int ini_read(struct ini * ini, FILE * fid);
void ini_write(struct ini * ini, FILE * fid);
void ini_del(struct ini * ini);
char const * ini_get(struct ini const * ini,
	char const * section, char const * name);
#endif
