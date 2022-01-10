
## Exercises

Lets look at a simple key-value store that needs to learn to be more careful about memory.
Above each function, we specify the ownership of pointers being passed -- either passing ownership, or borrowing the memory.
The current implementations do *not* adhere to these specifications.

```c
#include <string.h>
#include <malloc.h>

/*
 * Lets just use a single key/value as a proxy for an entire kv/store.
 * You can assume that the stored values are strings, so `strdup` can
 * allocate the memory for and copy the values. You absolutely will
 * have to use `strdup` in some of the functions.
 */
static int   kv_key;
static char *kv_value;

/**
 * ...
 *
 * The returned value is not maintained in the data-structure
 * and should be `free`d by the caller.
 */
char *
get_pass_ownership(int key)
{
	if (key != kv_key) return NULL;

	return kv_value;
}

/**
 * ...
 *
 * Pointers to the returned value are maintained in the data-structure
 * and it will be `free`d by the data-structure.
 */
char *
get_borrow(int key)
{
	if (key != kv_key) return NULL;

	return kv_value;
}

/**
 * ...
 *
 * Pointers to the `value` passed as the second argument are maintained by
 * the caller, thus the `value` is borrowed here. The `value` will be `free`d
 * by the caller.
 */
void
set_borrow(int key, char *value)
{
	/* What do we do with `kv_value`? Do we `strdup` anything? */
	kv_key   = key;
	kv_value = value;

	return;
}

/**
 * ...
 *
 * Pointers to the `value` passed as the second argument are not maintained by
 * the caller, thus the `value` should be `free`d by the caller.
 */
void
set_pass_ownership(int key, char *value)
{
	/* What do we do with `kv_value`? Do we `strdup` anything? */
	kv_key   = key;
	kv_value = value;

	return;
}

int
main(void)
{
	/* The values we pass in. */
	char *v_p2p = strdup("value1"); /* calls `malloc` ! */
	char *v_p2b = strdup("value2");
	char *v_b2p = strdup("value3");
	char *v_b2b = strdup("value4");

	/* The return values */
 char *r_p2p, *r_p2b, *r_b2p, *r_b2b;

	/* p2p: passing ownership on set, passing ownership on get */
	set_pass_ownership(0, v_p2p);
	r_p2p = get_pass_ownership(0);
	/* The question: should we `free(v_p2p)`?, `free(r_p2p)`? */

	/* p2b: passing ownership on set, borrowing memory for get */
	set_pass_ownership(0, v_p2b);
	r_p2b = get_borrow(0);
	/* The question: should we `free(v_p2b)`?, `free(r_p2b)`? */

	/* b2p: borrowing ownership on set, passing ownership on get */
	set_borrow(0, v_b2p);
	r_b2p = get_pass_ownership(0);
	/* The question: should we `free(v_b2p)`?, `free(r_b2p)`? */

	/* b2b: borrowing ownership on set, borrowing on get */
	set_borrow(0, v_b2b);
	r_b2b = get_borrow(0);
	/* The question: should we `free(v_b2b)`?, `free(r_b2b)`? */

	if (kv_value) free(kv_value);

	printf("Looks like success!...but wait till we valgrind; then ;-(\n");

	return 0;
}
```

The above code is hopelessly broken.
Run it in valgrind to see.

**Tasks:**

- In the above code, implement the `malloc`/`free`/`strdup` operations that are necessary both in the key-value implementation, and in the client (`main`) to make both the caller and callee abide by the memory ownership constraints.
- In which cases can *stack allocation* of the values be used in `main`?
    Why?
- In which cases can *stack allocation* of the values in the key-value store (i.e. in `get`/`set`) be used?
    Why?
