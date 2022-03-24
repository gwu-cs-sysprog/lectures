#include <unistd.h>
#include <stdio.h>

/*
 * Lets simplify malloc: lets only allocate memory of chunks of size
 * 128 - size of(struct memory_header) bytes.
 *
 * We'll use this same header data-structure to track both the
 * allocations, and freed memory. Because we're only allocating memory
 * of a single fixed size, we don't need to track how large an
 * allocation or free chunk is.
 */
struct memory_header {
	struct memory_header *next;
};

/* The head of the linked list of free chunks. */
struct memory_header *freelist;

/*
 * Find the first free chunk of memory on the freelist, remove it, and
 * return it
 */
void *
smalloc(void)
{
	struct memory_header *h = freelist;

	freelist = freelist->next;

	/* QUESTION 1: What is this doing? Why? Please draw it out. */
	return &h[1];
}

/*
 * We're passed the memory previously allocated. Find its header, and
 * add it to the freelist!
 */
void
sfree(void *m)
{
	/* QUESTION 2: What is this doing? Please draw it out. */
	struct memory_header *h = &((struct memory_header *)m)[-1];

	/*
	 * QUESTION 3: Add the code here to ensure that we can reuse
	 * the memory by adding the memory we're freeing back onto the
	 * freelist.
	 */
}

/*
 * Here we expand the heap, by making a system call to ask the kernel
 * for a large chunk of memory (using sbrk). Then we construct the
 * freelist of memory to use for future allocations.
 *
 * QUESTION 4: explain in your own terms what the following is doing.
 * Can you draw the data-structure out?
 */
#define HEAP_GROWTH (1 << 16)
void
expand_heap(void)
{
	struct memory_header *h;
	char *expanded_heap = sbrk(HEAP_GROWTH);
	char *mem;

	/*
	 * First job: create the *freelist* of memory chunks we can
	 * use for allocations. This is just a linked list of only the
	 * memory chunks that are not currently allocated. Thus it can
	 * be used to service allocation request.
	 */
	for (mem = expanded_heap; /* The first chunk of memory is the start of the grown heap */
	     mem + 128 < expanded_heap + HEAP_GROWTH; /* iterate through chunks until we're at the last chunk in the grown heap */
	     mem += 128) {			      /* iterate through chunks in memory */
		h = (struct memory_header *)mem;      /* treat chunk of memory as a header */
		h->next = (struct memory_header *)(mem + 128); /* ...pointing to the next chunk */
	}
	h = (struct memory_header *)mem;
	h->next = freelist;		/* The last chunk of memory has no next free chunk. */
	freelist = (struct memory_header *)expanded_heap;
}

#define N 4
int *mem[N];

int
main(void)
{
	int i;

	expand_heap();

	/* Allocate N chunks of memory, then free them! */

	printf("Lets initially allocate memory.\n");
	for (i = 0; i < N; i++) {
		mem[i] = smalloc();
		printf("\tAllocation @ %p\n", mem[i]);
	}
	printf("Then free it, so that we can reuse it.\n");
	for (i = 0; i < N; i++) {
		sfree(mem[i]);
	}
	printf("For these allocations, we should see the *same* addresses as before!\n");
	for (i = 0; i < N; i++) {
		mem[i] = smalloc();
		printf("\tAllocation @ %p\n", mem[i]);
	}

	return 0;
}
