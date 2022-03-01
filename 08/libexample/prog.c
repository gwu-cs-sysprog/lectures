#include <stdio.h>
#include <example.h>

int *
foo(int i)
{
	return bar(i + var);
}

int
main(void)
{
	int *v = foo(42);

	printf("%d and %d\n", *v, var);

	return 0;
}
