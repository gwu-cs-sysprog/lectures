#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

const int global_readonly = 0;
int global = 0;

int
main(void)
{
	int stack_allocated;
	int *heap = malloc(sizeof(int));

	printf("pid %d\nglobal (RO):\t%p\nglobal:\t%p\nstack:\t%p\nheap:\t%p\nfunction:\t%p\n",
	       getpid(), &global_readonly, &global, &stack_allocated, heap, main);

	pause();

	return 0;
}
