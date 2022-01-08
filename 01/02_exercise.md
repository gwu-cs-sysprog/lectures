
## Exercise: Quick-and-dirty Key-Value Store

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
struct kv_entry entries[NUM_ENTRIES]; /* global values are initialized to `0` */
size_t num_items = 0;

/**
 * Insert into the key-value store the `key` and `value`.
 * Return `0` on successful insertion of the value, or `-1` if the value couldn't be inserted.
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
		struct kv_entry *ent = lfind(&queries[i], entries, &num_items, sizeof(entries[0]), compare);
		int val = 0;

		if (ent != NULL) {
			val = ent->key;
		}

		printf("%d: %d @ %p\n", i, val, ent);
	}

	return 0;
}
```

You want to implement a simple "key-value" store that is very similar in API to a hash-table (many key-value stores are implemented using hash-tables!).

### Questions/Tasks

1. *Q1*: What is the difference between `lsearch` and `lfind`?
    What operations would you want to perform with each (and why)?
2. *Q2*: The current implementation doesn't include *values* at all.
    It returns the keys instead of values.
	Expand the implementation to properly track values.
3. *Q3*: Encapsulate the key-value store behind the `put` and `get` functions.
4. *Q4*: Add testing into the `main` for the relevant conditions and failures in `get` and `put`.
