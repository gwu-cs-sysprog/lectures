#include <stdio.h>

int
main(int argc, char *argv[])
{
	int i;

	printf("Inside %s\n", argv[0]);

	for (i = 0; i < argc; i++) {
		printf("arg %d: %s\n", i, argv[i]);
	}

	return 0;
}
