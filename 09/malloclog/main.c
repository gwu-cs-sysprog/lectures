#include <stdlib.h>

int
main(void)
{
	int i;
	int *p = malloc(sizeof(int));
	free(p);

	for (i = 0; i < 8; i++) {
		malloc(1024);
	}

	return 0;
}
