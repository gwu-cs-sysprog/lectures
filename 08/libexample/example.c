#include <example.h>
// #include <malloc.h>
#include <stdlib.h>

int var = 2;
/* do NOT do this in real code */
char li[] =
#include <li.txt>
;

int *
bar(int a)
{
	(void)li;

	int *p = calloc(1, sizeof(int));

	*p = a + var;

	return p;
}
