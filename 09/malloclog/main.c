#include <stdlib.h>

int
main(void)
{
	int *i = malloc(sizeof(int));
	free(i);

	return 0;
}
