#include <stdio.h>
#include <string.h>

#include <example.h>
#include <example2.h>

int *
foo(int i)
{
	return bar(i + var);
}

int
main(void)
{
	char *li = strdup(li2ptr);
	int *v = foo(42);

	li[12] = '\0';

	printf("%d and %d also with %s\n", *v, var, li);

	return 0;
}
