#include "iniparser.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
	struct ini * ini = ini_new();
	assert(ini != NULL);
	FILE * fid = fopen("testfile", "r");
	assert(fid != NULL);
	int res = ini_read(ini, fid);
	fclose(fid);
	assert(res == 0);
	ini_write(ini, stdout);
	return 0;
}