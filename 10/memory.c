#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define N 64

/* array of pointers to the memory we'll allocate */
int *mem[N];

int
main(void)
{
	int i;

	/* Allocate N chunks of memory, then free them! */

	for (i = 0; i < N; i++) {
		mem[i] = malloc(16);
	}
	for (i = 0; i < N; i++) {
		free(mem[i]);
	}

	return 0;
}
