#include <stdlib.h>

#define N  4
#define SZ 8

int
main(void)
{
	int i;
	/* array of pointers */
	int *mem[N] = {0};

	for (i = 0; i < N; i++) {
		mem[i] = malloc(SZ);
	}
	for (i = 0; i < N; i++) {
		free(mem[i]);
	}

	/* What about for other values of N and SZ? */
	/* What about if you malloc then immediately free the memory N times instead? */

	return 0;
}
