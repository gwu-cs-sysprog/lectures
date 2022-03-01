#include <example.h>
#include <malloc.h>

int var = 2;

int *
bar(int a)
{
	int *p = calloc(1, sizeof(int));

	*p = a + var;

	return p;
}
