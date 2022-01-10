
## Exercises

### C is a Thin Language Layer on Top of Memory

We're going to look at a set of variables as memory.
When variables are created globally, they are simply allocated into subsequent addresses.

```c
#include <stdio.h>
#include <string.h>

void print_values(void);

unsigned char a = 1;
int b = 2;
struct foo {
	long c_a, c_b;
	int *c_c;
};
struct foo c = (struct foo) { .c_a = 3, .c_c = &b };
unsigned char end;

int
main(void)
{
	size_t vars_size;
	unsigned int i;
	unsigned char *mem;

	/* Q1: What would you predict the output of &end - &a is? */
	printf("Addresses:\na   @ %p\nb   @ %p\nc   @ %p\nend @ %p\n"
	       "&end - &a = %ld\n", /* Note: you can split strings! */
		   &a, &b, &c, &end, &end - &a);
	printf("\nInitial values:\n");
	print_values();
	/* Q2: Describe what these next two lines are doing. */
	vars_size = &end - &a;
	mem = &a;
	/* Q3: What would you expect in the following printout? */
	printf("\nPrint out the variables as raw memory\n");
	for (i = 0; i < vars_size; i++) {
		unsigned char c = mem[i];
//		printf("%x ", c);
	}
	/* Q4: What would you expect in the following printout? */
	memset(&a, 0, vars_size);
	printf("\n\nPost-`memset` values:\n");
//	print_values()

	return 0;
}

void
print_values(void)
{
	printf("a     = %d\nb     = %d\nc.c_a = %ld\nc.c_b = %ld\nc.c_c = %p\n",
		   a, b, c.c_a, c.c_b, c.c_c);
}
```

**Question**
Answer *Q1-4* in the code, then modify the code where appropriate to investigate them.

#### Takeaways

1. Each variable in C (including fields in structs) want to be aligned on a boundary equal to the variable's type's size.
    This means that a variable (`b`) with an integer type (`sizeof(int) == 4`) should always have an address that is a multiple of its size (`&b % sizeof(b) == 0`, so an `int`'s address is always divisible by `4`, a `long`'s by `8`).
2. The operation to figure out the size of all the variables, `&end - &a`, is *crazy*.
    We're used to performing math operations values on things of the same type, but not on *pointers*.
	This is only possible because C sees the variables are chunks of memory that happen to be laid out in memory, one after the other.
3. The *crazy* increases with `mem = &a`, and our iteration through `mem[i]`.
	We're able to completely ignore the types in C, and access memory directly!

**Question**: What would break if we changed `char a;` into `int a;`?
C doesn't let us do math on variables of *any type*.
If you fixed compilation problems, would you still get the same output?

### Quick-and-dirty Key-Value Store

Please read the `man` pages for `lsearch` and `lfind`.
`man` pages can be pretty cryptic, and you are aiming to get *some idea* where to start with an implementation.
An simplistic, and incomplete initial implementation:

```c
#include <stdio.h>
#include <assert.h>
#include <search.h>

#define NUM_ENTRIES 8
struct kv_entry {
	int key; /* only support keys for now... */
};
/* global values are initialized to `0` */
struct kv_entry entries[NUM_ENTRIES];
size_t num_items = 0;

/**
 * Insert into the key-value store the `key` and `value`.
 * Return `0` on successful insertion of the value, or `-1`
 * if the value couldn't be inserted.
 */
int
put(int key, int value)
{
	return 0;
}

/**
 * Attempt to get a value associated with a `key`.
 * Return the value, or `0` if the `key` isn't in the store.
 */
int
get(int key)
{
	return 0;
}

int
compare(const void *a, const void *b)
{
	/* We know these are `int`s, so treat them as such! */
	const struct kv_entry *a_ent = a, *b_ent = b;

	if (a_ent->key == b_ent->key) return 0;

	return -1;
}

int
main(void)
{
	struct kv_entry keys[] = {
		{.key = 1},
		{.key = 2},
		{.key = 4},
		{.key = 3}
	};
	int num_kv = sizeof(keys) / sizeof(keys[0]);
	int queries[] = {4, 2, 5};
	int num_queries = sizeof(queries) / sizeof(queries[0]);
	int i;

	/* Insert the keys. */
	for (i = 0; i < num_kv; i++) {
		lsearch(&keys[i], entries, &num_items, sizeof(entries[0]), compare);
	}

	/* Now lets lookup the keys. */
	for (i = 0; i < num_queries; i++) {
		struct kv_entry *ent;
		int val = 0;

		ent = lfind(&queries[i], entries, &num_items, sizeof(entries[0]), compare);
		if (ent != NULL) {
			val = ent->key;
		}

		printf("%d: %d @ %p\n", i, val, ent);
	}

	return 0;
}
```

You want to implement a simple "key-value" store that is very similar in API to a hash-table (many key-value stores are implemented using hash-tables!).

**Questions/Tasks:**

- *Q1*: What is the difference between `lsearch` and `lfind`?
    What operations would you want to perform with each (and why)?
- *Q2*: The current implementation doesn't include *values* at all.
    It returns the keys instead of values.
	Expand the implementation to properly track values.
- *Q3*: Encapsulate the key-value store behind the `put` and `get` functions.
- *Q4*: Add testing into the `main` for the relevant conditions and failures in `get` and `put`.
