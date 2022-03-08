#define _GNU_SOURCE /* added this due to `man 3 dlsym` */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <assert.h>

typedef void *(*malloc_fn_t)(size_t s);
typedef void  (*free_fn_t)(void *);
static malloc_fn_t m;
static free_fn_t f;
static int in_initialization = 0;

/*
 * Lets find the malloc and free functions in libc! The `RTLD_NEXT`s
 * specify that we don't want to find our *own* `malloc`, we'd like to
 * search starting with the *next* library.
 */
static void
initialize(void)
{
	in_initialization = 1;
	/* Lets find libc's free and malloc! */
	f = dlsym(RTLD_NEXT, "free");
	m = dlsym(RTLD_NEXT, "malloc");
	assert(f && m);
	in_initialization = 0;
}

/* hackey malloc while initializing */
static void *init_malloc(size_t sz);

void *
malloc(size_t sz)
{
	void *mem;

	/* if we don't yet know where libc's malloc is, look it up! */
	if (m == NULL) {
		/* If the `dlsym` in `initialize` calls malloc, we have to handle it! */
		if (in_initialization) return init_malloc(sz);
		initialize();
	}
	mem = m(sz);
	fprintf(stderr, "malloc(%ld) -> %p\n", sz, mem);

	return mem;
}

void
free(void *mem)
{
	/* if we don't yet know where libc's free is, look it up! */
	if (f == NULL) {
		if (in_initialization) return;
		initialize();
	}
	f(mem);
	fprintf(stderr, "free(%p)\n", mem);
}

static char init_mem[1024 * 8];
static unsigned long init_off = 0;

/*
 * While we're initializing, a call to malloc in `dlsym` requires
 * memory, thus will call `malloc`! But we need `dlsym` to figure out
 * where `malloc` is. How do we break this circular dependency?
 *
 * This is the simplest allocator (called a bump-allocator) to solve
 * this problem. It is only used when we're initializing to satisfy
 * `dlsym`'s `malloc` requests.
 */
static void *
init_malloc(size_t sz)
{
	char *ret;
	size_t i;

	ret = &init_mem[init_off];
	for (i = 0; i < sz; i++) ret[i] = 0;
	init_off += sz;
	if (init_off >= sizeof(init_mem)) exit(EXIT_FAILURE);

	return ret;
}
