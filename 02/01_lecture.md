
# C Memory Model, Data-structures, and APIs

C presents some unique opportunities for how to structure your code and data-structures, but also requires that you're careful about how data is passed around a program.

## Memory Allocation Options

In C, when creating a variable, you have the option of allocating it in one of multiple different types of memory:

- In another existing structure.
- In the heap using `malloc` or its sibling functions.
- In global memory.
- On the stack.

It might be surprising, but it is quite *uncommon* in programming languages to have this flexibility.

### Internal Allocation

What does it look like to do internal allocation of one struct or array inside of another?
See the following as an example.

```c
#include <stdlib.h>

struct bar {
	/*
	 * `arr` is allocated internally to `bar`, whereas `bytes` is a pointer to
	 * a separate allocation. `arr` must have a *fixed size*, and `bytes` does not
	 * because its allocation can be as large as you want!
	 */
	int arr[64];
	unsigned char *bytes;
};

struct foo {
	/* The `bar` structure is allocated as *part of* the `struct foo` in `internal` */
	struct bar internal;
	/* But we can *also* allocate another `bar` separately if we'd prefer */
	struct bar *external;
};

int
main(void)
{
	/*
	 * We didn't have to separately allocate the `struct bar internal` as
	 * it was allocated with the enclosing `a` allocation. However, we do have to
	 * allocate the `external` allocation. In both cases, we have access to a
	 * `struct bar` from within `struct foo`.
	 */
	struct foo *a = malloc(sizeof(struct foo)); /* should check return value */
	a->external   = malloc(sizeof(struct bar)); /* and this one ;-( */

	/*
	 * Note the difference in needing to use `->` when `external` is a pointer
	 * versus simply using `.` with `internal`.
	 */
	a->internal.arr[0] = a->external->arr[0];

	return 0;
}
```

One of the more interesting uses of internal allocation is in linked data-structures (e.g. like linked lists).
It is common in languages like java to implement linked data-structures to have "nodes" that reference the data being tracked.

```java
// This could be made generic across the data types it could store by instead
// using `class LinkedList<T>`, but I'm keeping it simple here.
class LinkedListOfStudents {
	Node head;

	class Node {
		Student s;
		Node next;

		Node(Student s, Node next) {
			this.s    = s;
			this.next = next;
		}
	}

	void add(Student s) {
		// The program has *previously* allocated `data`.
		// Now we have to additionally allocate the `node` separate from the data!
		Node n = new(s, this.head);

		this.head = n;
	}

	// ...
}
// This looks like the following. Note separate allocations for Node and Student
//
// head --> Node------+    ,-->Node-----+
//          | s  next |---'    | s next |---...
//          +-|-------+        +-|------+
//            v                  V
//          Student-+           Student-+
//          | ...   |           | ...   |
//          +-------+           +-------+
```

Lets see the same in C.

```c
#include <stdlib.h>
#include <stdio.h>

struct linked_list_node {
	void *data;
	struct linked_list_node *next;
};

struct linked_list {
	struct linked_list_node *head;
};

struct student {
	// student data here...
	struct linked_list_node list;
};

void
add(struct linked_list *ll, struct linked_list_node *n, void *data)
{
	n->next  = ll->head;
	n->data  = data;
	ll->head = n;
}

struct linked_list l;

int
main(void)
{
	struct student *s = malloc(sizeof(struct student));
	/* Should check that `s != NULL`... */
	add(&l, &s->list, s); /* note that `&s->list` is the same as `&(s->list)` */

	printf("student added to list!\n");

	return 0;
}

/*
 * This looks something like the following. Linked list is *internal* to student.
 *
 * head ----> student-+     ,-> student-+
 *            | ...   |    ,    | ...   |
 *            | next  |---'     | next  |--> NULL
 *            +-------+         +-------+
 */
```

A few interesting things happened here:

1. We're using `void *` pointers within the `linked_list_node` so that the linked list can hold data of *any pointer type*.
    You can see that in our list implementation, there is nothing that is `student`-specific.
2. The `list` is inline-allocated inside the `student` structure, which completely avoids the separate `node` allocation.

Most serious data-structure implementations enable this inlining of data-structure nodes even without requiring the `data` pointer in the node.
We won't cover that, but you can [see a sample implementation](https://github.com/gwsystems/ps/blob/master/ps_list.h).

### Heap Allocation

We've already seen `malloc`, `calloc`, `realloc`, and `free` which are our interface to the heap.
These provide the most flexibility, but require that you track the memory and `free` it appropriately^[Not always an easy feat.
See the previous discussion on Common Errors.]

### Global Allocation

We have already seen global allocations frequently in examples so far.

```c
#include <stdio.h>

/* A globally allocated array! No `malloc` needed! */
int arr[64];
struct foo {
	int a, b;
	struct foo *next;
};
/* A globally allocated structure! */
struct foo s;

/* Globally allocated *and* initialized integer... */
int c = 12;
/* ...and array. */
long d[] = {1, 2, 3, 4};

int
main(void)
{
	printf("What is uninitialized global memory set to?\n"
	       "Integer: %d\nPointer: %p (as hex: %lx)\n",
	       arr[0], s.next, (unsigned long)s.next);
	/* Note that we use `.` to access the fields because `s` is not a pointer! */

	return 0;
}
```

Global variables are either initialized where they are defined, or are initialized to `0`.
Note that they are intialized to all `0`s regardless their type.
This makes more sense than it sounds because pointers set to `0` are `NULL` (because, recall, `NULL` is just `(void *)0` -- see the "hex" output above), and because strings (see later) are terminated by `\0` which is also `0`!
Thus, this policy initializes all numerical data to `0`, all pointers to `NULL`, and all strings to `""`.

### Stack Allocation

Variables can also be allocated *on the stack*.
This effectively means that as you're executing in a function, variables can be allocated within the context/memory of that function.
An example that allocates a structure and an array on the stack:

```c
#include <stdio.h>
#include <assert.h>

#define ARR_SZ 12

/*
 * Find an integer in the array, reset it to `0`, and return its offset.
 * Nothing very interesting here.
 */
int
find_and_reset(int *arr, int val)
{
	int i;

	/* find the value */
	for (i = 0; i < ARR_SZ; i++) {
		if (arr[i] == val) {
			arr[i] = 0;
			return i;
		}
	}
	/* Couldn't find it! */
	return -1;
}

int
fib(int v)
{
	/* Allocate an array onto the stack */
	int fibs[ARR_SZ] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89}; /* looks like a suspicious sequence... */
	int ret;

	ret = find_and_reset(fibs, v);
	/* should have been set to `0`, so this should return `-1` */
	assert(find_and_reset(fibs, v) == -1);

	return ret;
}

int
main(void)
{
	printf("Should find 8 @ 6: %d\n", fib(8));
	/* if the array was the same array for these two calls, it would be found at offset -1 (error) */
	printf("Should find 8 @ 6: %d (-1 is wrong here)\n", fib(8));

	return 0;
}
```

Key points to realize:

- `fibs` in `fib` is allocated *on the stack* and is initialized in `fib`!
- We are passing the array into `find_and_reset` which modifies the array directly as it is passed as a pointer.
- The first time that `fib` is called, it creates the `arr`.
    After we *return* from `fib`, the `fibs` goes away (strictly: its memory is reclaimed as part of returning from `fib`).
- The second time we call `fib`, it effectively is a *new* execution of `fib`, thus a *new* allocation of `fibs` that is now initialized a second time.

#### Stack Usage Example

How do we think about this?
The stack starts out in `main`:

```
stack          |
|              |
+-main---------+
|              | <--- main's local data (note: no local variables)
+--------------+
```

What we're drawing here is a "*stack frame*" for the `main` function.
This includes all of the local data allocated within the function, and (at least conceptually) also includes the arguments passed into the function (`main` has none).
When it calls `fib`, a stack frame is allocated for `fib`, the argument is passed in, and `fib`s variables are allocated:

```
stack          |
|              |
+-main---------+
|              |
+-fib----------+
| arg: v       | <--- argument to fib
| fibs[ARR_SZ] | <-+- local variables, allocated here!
| ret          | <-'
+--------------+
```

`fib` calls `find_and_reset`:

```
stack          |
|              |
+-main---------+
|              |
+-fib----------+
| v            |
| fibs[ARR_SZ] |<--+
| ret          |   | pointer to fibs passed as argument, and used to update arr[v]
+find_and_reset+   |
| arg: arr     |---+
| arg: val     |
| i            |
+--------------+
```

Since `find_and_reset` updates the array in `fib`'s stack frame, when it returns, the array is still properly updated.
Importantly, once we return to `main`, we have deallocated all of the variables from the previous call to `fib`...

```
stack          |
|              |
+-main---------+
|              | <--- returning to main, deallocates the fibs array in fib
+--------------+
```

...thus the next call to `fib` allocates a *new* set of local variables including `fibs`.

```
stack          |
|              |
+-main---------+
|              |
+-fib----------+
| arg: v       |
| fibs[ARR_SZ] | <--- new version of fibs, re-initialized when we fib is called
| ret          |
+--------------+
```

#### Common Errors in Stack Allocation

A few common errors with stack allocation include:

- *Uninitialized variables.*
    You must *always* initialize all variables allocated on the stack, otherwise they can contain seemingly random values (see below).
- *Pointers to stack allocated variables after return.*
    After a function returns, its stack allocated variables are no longer valid (they were deallocated upon `return`).
	Any pointers that remain to any of those variables are no longer pointing to valid memory!

We discuss these next.

**Initializing stack-allocated variables.**
For global memory, we saw that variables, if not intentionally initialized, would be set to `0`.
This is *not* the case with stack allocated variables.
In fact, the answer is "it depends".
If you compile your code without optimizations, stack allocated variables will be initialized to `0`; if you compile with optimizations, they are *not* initialized.
Yikes.
Thus, we must assume that they are not automatically initialized (similar to the memory returned from `malloc`).
An example:

```c
#include <stdio.h>

void
foo(void)
{
	/* don't manually initialize anything here */
	int arr[12];
	int i;

	for (i = 0; i < 12; i++) {
		printf("%d: %d\n", i, arr[i]);
	}
}

int
main(void)
{
	foo();

	return 0;
}
```

Yikes.
We're getting random values in the array!
Where do you think these values came from?

**References to stack variables after function return.**

```c
#include <stdio.h>

unsigned long *ptr;

unsigned long *
bar(void)
{
	unsigned long a = 42;

	return &a;            /* Return the address of a local variable. */
}

void
foo(void)
{
	unsigned long a = 42; /* Allocate `a` on the stack here... */

	ptr = &a;             /* ...and set the global pointer to point to it... */

	return;               /* ...but then we deallocate `a` when we return. */
}

int
main(void)
{
	unsigned long val;

	foo();
	printf("Save address of local variable, and dereference it: %lu\n", *ptr);
	fflush(stdout); /* ignore this ;-) magic sprinkles here */
	ptr = bar();
	printf("Return address of local variable, and dereference it: %lu\n", *ptr);

	return 0;
}
```

You can see a few interesting facts from the output.

1. The value referenced by `*ptr` after `foo` is a random value, and dereferencing the return value from`bar` causes a segmentation fault.
2. `foo` and `bar` contain logic that feels pretty identical.
    In either case, they are taking a local variable, and passing its address to `main` where it is used.
	`foo` passed it through a global variable, and `bar` simply returns the address.
	Despite this, one of them causes a segmentation fault, and the other seems to return a nonsensical value!
	When you try and use stack allocated variables after they are been freed (by their function returning), you get *unpredictable* results.
3. The C compiler is aggressive about issuing warnings as it can tell that bad things are afoot.
    Warnings are your friend and you *must* do your development with them enabled.

Stack allocation is powerful, can be quite useful.
However, you have to always be careful that stack allocated variables are never added into global data-structures, and are never returned.

### Putting it all Together

Lets look at an example that uses inlined, global, and stack allocation.
We'll avoid heap-based allocation to demonstrate the less normal memory allocation options in C.

```c
#include <stdio.h>
#include <search.h>
#include <assert.h>

struct kv_entry {
	unsigned long key;
	char *value;
};

struct kv_store {
	/* internal allocation of the key-value store's entries */
	struct kv_entry entries[16];
	size_t num_entries;
};

int
kv_comp(const void *a, const void *b)
{
	const struct kv_entry *a_ent = a;
	const struct kv_entry *b_ent = b;

	/* Compare keys, and return `0` if they are the same */
	return !(a_ent->key == b_ent->key);
}

int
put(struct kv_store *store, unsigned long key, char *value)
{
	/* Allocate a structure on the stack! */
	struct kv_entry ent = {
		.key = key,
		.value = value
	};
	struct kv_entry *new_ent;
	/* Should check if the kv_store has open entries. */

	/* Notice we have to pass the `&ent` as a pointer is expected */
	new_ent = lsearch(&ent, store->entries, &store->num_entries, sizeof(struct kv_entry), kv_comp);
	/* Should check if we found an old entry, and we need to update its value. */

	if (new_ent == NULL) return -1;

	return 0;
}

int
main(void)
{
	/* Allocated the data-store on the stack, including the array! */
	struct kv_store store = { .num_entries = 0 };

	unsigned long key = 42;
	char *value = "secret to everything";
	put(&store, key, value);

	/* Validate that the store got modified in the appropriate way */
	assert(store.num_entries == 1);
	assert(store.entries[0].key == key);

	printf("%ld is the %s\n", store.entries[0].key, store.entries[0].value);

	return 0;
}
```

In this program you might notice that we've used *no dynamic memory allocation at all*!
The cost of this is that we had to create a key-value store of only a fixed size (`16` items, here).

### Comparing C to Java

C enables a high-degree of control in which memory different variables should be placed.
Java traditionally has simple rules for which memory is used for variables:

- *[Primitive types](https://docs.oracle.com/javase/tutorial/java/nutsandbolts/datatypes.html)* are stored in objects and on the stack.
- *Objects* are always allocated using `new` in the heap, and references to objects are always pointers.
- The heap is managed by the garbage collector, thus avoiding the need for `free`.

Many aspects of this are absolutely fantastic:

- The programmer doesn't need to think about how long the memory for an object needs to stick around -- the garbage collector instead manages deallocation.
- The syntax for accessing fields in objects is uniform and simple: always use a `.` to access a field (e.g. `obj.a`) -- all object references are pointers, so instead of having `->` everywhere, just replace it with the uniform `.`.

However, there are significant downsides when the goal is to write systems code:

- Some systems don't have a heap!
    Many embedded (small) systems don't support dynamic allocation.
- Garbage collection (GC) isn't free!
	It has some overhead for some applications, can result in larger memory consumption (as garbage is waiting to be collected), and can causes delays in processing when GC happens.
	That said, modern GC is pretty amazing and does a pretty good job at minimizing all of these factors.
- Many data-structures that might be allocated globally or on the stack, instead must be allocated on the heap, which is slower.
	Similarly, many objects might want other objects to be part of their allocation, but that isn't possible as each must be a separate heap allocation, which adds overhead.

## Strings

String don't have a dedicated type in C -- there is no `String` type.
Strings are

1. arrays of `char`s,
2. with a null-terminator (`\0`) character in the last array position to denote the termination of the string.

In memory, a simple string has this representation:

```
char *str = "hi!"

str ---> +---+---+---+---+
         | h | i | ! |\0 |
         +---+---+---+---+
```

Note that `\0` is a single character even though it looks like two.
We can see this:

```c
#include <stdio.h>
#include <assert.h>

int
main(void)
{
	int i;
	char *str = "hi!";
	char str2[4] = {'h', 'i', '!', '\0'};        /* We can allocate strings on the stack! */

	for (i = 0; str[i] != '\0'; i++) {           /* Loop till we find the null-terminator */
		assert(str[i] == str2[i]);               /* Verify the strings are the same. */
		printf("%c", str[i]);
	}
	assert(str[3] == str2[3] && str[3] == '\0'); /* Explicitly check that the null-terminator is there */
	printf("\n");
}
```

So strings aren't really all that special in C.
They're just arrays with a special terminating character, and a little bit of syntax support to construct them (i.e. `"this syntax"`).
So what's there to know about strings in C?

### `string.h` Functions

Working with strings is *not* C's strong point.
However, you have to handle strings in every language, and C provides a number of functions to do so.
You can read about each of these `function`s with `man 3 <function>`.

#### Core String Operations

- `strlen` - How many characters is a string (not including the null-terminator)?
- `strcmp` - Compare two strings, return `0` if they are the same, or `-1` or `1`, depending on which is [lexographically less than the other](https://en.wikipedia.org/wiki/Lexicographic_order).
    Similar in purpose to `equals` in Java.
- `strcpy` - Copy into a string the contents of another string.
- `strcat` - Concatenate, or append onto the end of a string, another string.
- `strdup` - Duplicate a string by `malloc`ing a new string, and copying the string into it (you have to `free` the string later!).
- `snprintf` - You're familiar with `printf`, but `snprintf` enables you to "print" into a string!
    This gives you a lot of flexibility in easily constructing strings.
	A downside is that you don't really know how big the resulting string is, so the `n` in the name is the maximum length of the string you're creating.

```c
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	char result[256] = {'\0', };                /* initialize string to be "" */
	char *a = "hello", *b = "world";
	int c = 42;
	int ret;
	char *d;

	assert(strcmp(a, b) != 0);                  /* these strings are different */

	strcat(result, a);
	strcat(result, " ");
	strcat(result, b);
	assert(strcmp(result, "hello world") == 0); /* should have constructed this string properly */

	d = strdup(result);
	assert(strcmp(d, result) == 0);             /* a duplicate should be equal */
	free(d);

	strcpy(result, "");                         /* reset the `result` to an empty string */

	ret = snprintf(result, 255, "%s %s and also %d", a, b, c);
	printf("\"%s\" has length %d\n", result, ret);
}
```

Many of these functions raise an important question:
What happens if one of the strings is not large enough to hold the data being put into it?
If you want to `strcat` a long string into a small character array, what happens?
This leads us to a simple fact...

**It is *easy* to use the string functions incorrectly.**
Many of these functions also have a `strnX` variant where `X` is the operation (`strnlen`, `strncmp`, etc..).
These are safer variants of the string functions.
The key insight here is that if a string is derived from a user, it might not actually be a proper string!
It might not, for example, have a null-terminator -- uh oh!
In that case, many of the above functions will keep on iterating *past* the end of the buffer

```c
#include <string.h>
#include <stdio.h>

char usr_str[8];

int
main(void)
{
	char my_str[4];

	/*
	 * This use of `strcpy` isn't bugged as we know the explicit string is 7 zs,
	 * and 1 null-terminator, which can fit in `usr_str`.
	 */
	strcpy(usr_str, "zzzzzzz");

	/*
	 * Also fine: lets limit the copy to 3 bytes, then add a null-terminator ourself
	 * (`strlcpy` would do this for us)
	 */
	strncpy(my_str, usr_str, 3);
	my_str[3] = '\0';

	printf("%s\n", my_str);
	fflush(stdout);  /* don't mind me, making sure that your print outs happen */

	/*
	 * However, note that `strlen(usr_str)` is larger than the size of `my_str` (4),
	 * so we copy *past* the buffer size of `my_str`. This is called a "buffer overflow".
	 */
	strcpy(my_str, usr_str);

	return 0;
}
```

#### Parsing Strings

While many of the previous functions have to do with creating and modifying strings, computers frequently need to "parse", or try to understand the different parts of, a string.
Some examples:

- The code we write in `.c` or `.java` files is just a long string, and the programming language needs to *parse* the string to understand what commands you're trying to issue to the computer.
- The shell must *parse* your commands to determine which actions to perform.
- Webpages are simply a collection of `html` code (along with other assets), and a browser needs to *parse* it to determine what to display.
- The markdown text for this lecture is *parsed* by `pandoc` to generate a `pdf` and `html`.

Some of the core functions that help us parse strings include:

- `strtol` - Pull an integer out of a string.
    Converts the first part of a string into a `long int`, and also returns an `endptr` which points to the character *in* the string where the conversion into a number stopped.
	If it cannot find an integer, it will return `0`, and `endptr` is set to point to the start of the string.
- `strtok` - Iterate through a string, and find the first instance of one of a number of specific characters, and return the string leading up to that character.
    This is called multiple times to iterate through the string, each time extracting the substring up to the specific characters.
	See the example in the `man` page for `strtok` for an example.
- `strstr` - Try and find a string (the "needle") in another string (the "haystack"), and return a pointer to it (or `NULL` if you don't find it).
    As such, it finds a string in another string (thus `strstr`).
- `sscanf` -
    A versatile function will enables a format string (e.g. as used in `printf`) to specify the format of the string, and extract out digits and substrings.

Some examples:

```c
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int
main(void)
{
	char *input = "1 42 the secret to everything";
	char *inputdup;
	char *substr;
	char *substr_saved;
	long int extracted;
	char sec[32];

	extracted = strtol(input, &substr, 10);                               /* pull out the first two integers */
	printf("extracted %ld, remaining string: \"%s\"\n", extracted, substr);
	extracted = strtol(substr, &substr, 10);
	printf("extracted %ld, remaining string: \"%s\"\n", extracted, substr);
	substr_saved = substr;

	/* what happens when we cannot extract a long? */
	extracted = strtol(substr_saved, &substr, 10);
	assert(extracted == 0 && substr_saved == substr);                     /* verify that we couldn't extract an integer */

	assert(strcmp(strstr(input, "secret"), "secret to everything") == 0); /* find secret substring */

	sscanf(input, "1 %ld the %s to everything", &extracted, sec);         /* extract out the number and the secret */
	printf("%ld and \"%s\"\n", extracted, sec);

	printf("Using strtok to parse through a string finding substrings separated by 'h' or 't':\n");
	inputdup = strdup(input);                                            /* strtok will modify the string, lets copy it */
	for (substr = strtok(inputdup, "ht"); substr != NULL; substr = strtok(NULL, "ht")) {
		printf("[%s]\n", substr);
	}

	return 0;
}
```

### Bonus: Explicit Strings

When you use an explicit string (e.g. `"imma string"`) in your code, you're actually asking C to allocate the string in global memory.
This has some strange side-effects:

```c
#include <stdio.h>
#include <string.h>

char c[5];

int
main(void)
{
	char *a = "blah";
	char *b = "blah";

	strncpy(c, a, 5);

	printf("%s%s%s\n", a, b, c);
	/* compare the three pointers */
	printf("%p == %p != %p\n", a, b, c);

	return 0;
}
```

The C compiler and linker are smart enough to see that if you have already used a string with a specific value (in this case `"clone"`), it will avoid allocating a copy of that string, and will just reuse the previous value.
Generally, it doesn't make much sense to look at the address of strings, and certainly you should not compare them.
You can see in this example how you must compare strings for equality using `strncmp`, and *not* to compare pointers.

## API Design and Concerns

[Slides](https://sibin.github.io/teaching/csci2410-gwu-systems_programming/fall_2023/slides/reveal_slides/api_design.html#/)

When programming in C, you'll see quite a few APIs.
Throughout the class, we'll see quite a few APIs, most documented in `man` pages.
It takes some practice in reading `man` pages to get what you need from them.
One of the things that helps the most is to understand a few common *patterns* and *requirements* that you find these APIs, and in C programming in general.

### Return Values

Functions often need to return multiple values.
C does not provide a means to return more than one value, thus is forced to use pointers.
To understand this, lets look at the multiple ways that pointers can be used as function arguments.

```c
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*
 * `arg` is used to pass an argument that happens to be an array here. In contrast,
 * `ret` is a *second return value*. This function will set the value that `ret` points
 * to -- which happens to be `retval` in the `main` stack frame -- to the value we are
 * getting from the array.
 */
int
get(int *arg, int offset, int *ret)
{
	if (arg == NULL) return -1;

	*ret = arg[offset];

	return 0;
}

/*
 * Again, the array is passed in as the first argument, but this time it is used to
 * store the new value.
 */
int
set(int *ret_val, int offset, int value)
{
	if (ret_val == NULL) return -1;

	ret_val[offset] = value;

	return 0;
}

/*
 * `arrdup`'s job is to duplicate an array by allocating and populating
 * a new array. It will return `0` or not `0` on success/failure. Thus
 * the new array must be returned using pointer arguments. `ret_allocated`
 * is a pointer to a pointer to an array in the calling function, and it
 * is used to return the new array.
 */
int
arrdup(int **ret_allocated, int *args, size_t args_size)
{
	size_t i;
	int *newarr;

	if (ret_allocated == NULL) return -1;

	newarr = calloc(args_size, sizeof(int)); /* 1 below */
	if (newarr == NULL) return -1;

	for (i = 0; i < args_size; i++) {
		newarr[i] = args[i];
	}
	*ret_allocated = newarr; /* 2 and 3 below */

	return 0;
}
/*
 * Lets draw this one. The stack setup when we call `arrdup`:
 *
 * |               |
 * +-main----------+
 * | arr           |<---+
 * | dup           |<-+ |
 * | ...           |  | |
 * +-arrdup--------+  | |
 * | ret_allocated |--+ |
 * | args          |----+
 * | ...           |
 * +---------------+
 *
 * `ret_allocated` points to `dup` in `main`.
 *
 *
 *    3. *ret_allocated = newarr
 *                          ^
 *                          |
 *            ,-------------'
 *           |
 * |         |     |
 * +-main----|-----+
 * | arr     |     |
 * | dup ---'  <------+
 * | ...           |  | -- 2. *ret_allocated
 * +-arrdup--------+  |
 * | ret_allocated ---+
 * | args          |
 * | newarr --------------> 1. calloc(...)
 * +---------------+
 *
 * 1. `arrdup` calls `calloc` to allocate on the heap
 * 2. Dereferencing `ret_allocated` gives us access to `dup`
 * 3. thus we can set dup equal to the new heap memory
 *
 * This effectively enables us to return the new memory into the
 * `dup` variable in main.
 */



int
main(void)
{
	int arr[] = {0, 1, 2, 3};
	int *dup;
	int retval;

	assert(get(arr, 2, &retval) == 0 && retval == 2);
	assert(set(arr, 2, 4) == 0);
	assert(get(arr, 2, &retval) == 0 && retval == 4);
	assert(arrdup(&dup, arr, 4) == 0);
	assert(get(dup, 2, &retval) == 0 && retval == 4);
	free(dup);

	printf("no errors!");

	return 0;
}
```
<!-- COMBINED ERROR NOTES WITH NEW MATERIAL IN ANOTHER FILE BY SIBIN in OCT 2023!

### Errors

The first question is how we can detect that some error occurred within a function we have called?
We'll separate functions into two different classes:

1. *Functions that return pointers.*
    Functions that return pointers (e.g. that have declarations of the form `type *fn(...)`) are often relatively straightforward.
	If they return `NULL`, then an error occurred; otherwise the returned pointer can be used.
	`malloc` is an example here.
2. *Functions that return integers.*
    Integers are used as a relatively flexible indication of the output of a function.
	Is it common for a `-1` to indicate an error.
	Sometimes *any* negative value indicates an error, each negative value designating that a different error occurred.
	Non-negative values indicate success.
	If a function wishes to return a binary success or failure, you'll see that many APIs (counter-intuitively) return `0` for success, and `-1` for failure.

It is common that you want more information than the return value can give you.
You want more information about *why* the failure happened so that you can debug more easily.
The `errno` variable is UNIX's solution to this (see its `man` page), and can be referenced if you include `errno.h`.
`errno` is simply an integer where specific values represent specific errors.
You can view these values by looking them up in the source^[For example, in `/usr/include/asm/errno.h`.] or, you can ask the `errno` *program*^[You might need to do `apt-get install moreutils` if you're using your own system.].
For example:

```
$ errno -l
EPERM 1 Operation not permitted
ENOENT 2 No such file or directory
ESRCH 3 No such process
EINTR 4 Interrupted system call
EIO 5 Input/output error
...
```

You can look up specific values:

```
$ errno 28
ENOSPC 28 No space left on device
```

You can imagine how that error might occur.
What if you're trying to add files on disk, and the disk runs out of room!?

If you want your program to print a useful error when you encounter such a problem, you can use the `perror` function (see its `man` page for documentation) to print out an error, or `strerror` (via `string.h`) if you just want a string corresponding to the error.

```c
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int
main(void)
{
	char *ret;

	printf("Lets get greedy: allocate %ld bytes!\n", LONG_MAX);
	ret = malloc(LONG_MAX);
	if (ret == NULL) {
		printf("Error: errno value %d and description: %s\n", errno, strerror(errno));
		fflush(stdout);
		perror("Error allocating memory");

	    return -1;
	}

	return 0;
}
```

(Note: when you return from a program with a non-zero value, it designates that your *program* had an error.
This is why we see the `make` error when it runs your program.)

To understand the return value of UNIX library functions:

- look at the function return values to understand the type (and identify if it is returning an `int` or a pointer)
```
SYNOPSIS
       #include <stdlib.h>

       void *malloc(size_t size);
	   ...
```
- Read through the description of the function(s).
- Read through the `RETURN VALUE` and `ERRORS` sections of the man page.
```
RETURN VALUE
       The malloc() and calloc() functions return a pointer to the allocated memory, which is  suitably  aligned  for
       any  built-in type.  On error, these functions return NULL.  NULL may also be returned by a successful call to
       malloc() with a size of zero, or by a successful call to calloc() with nmemb or size equal to zero.
...
ERRORS
       calloc(), malloc(), realloc(), and reallocarray() can fail with the following error:

       ENOMEM Out  of  memory.
	   ...

```

This explains why we got the error we did.
-->

### Memory Ownership

One of the last, but most challenging aspects of APIs in C is that of memory ownership.
The big question is: when a pointer passed into a function, or returned from a function, who is responsible for `free`ing the memory?
This is due to a combination of factors, mainly:

1. C requires that memory is explicitly `free`d, so someone has to do it, and it should be `free`d *only once*, and
2. pointers are passed around freely and frequently in C, so somehow the function caller
and callee have to understand who "owns" each of those pointers.

It is easiest to understand this issue through the concept of *ownership*: simply put, the owner of a piece of memory is the one that should either free it, or pass it to another part of the code that becomes the owner.
In contrast, a caller can pass a pointer to memory into a function allowing it to *borrow* that memory, but the function does *not* free it, and after it returns it should not further access the memory.
There are three general patterns:

- A caller *passes a pointer* into a function, and *passes the ownership* of that data to the function.
   This is common for data-structures whose job is to take a pointer of data to store, and at that point, they own the memory.
   Think: if we `enqueue` data into a queue.

   **Examples**: The key-value store's `put` function owns the passed in data (assuming it takes a `void *`.

- A function *returns a pointer* to the function caller and *passes the ownership* to the caller.
   The caller must later `free` the data.
   This is also common in data-structures when we wish to retrieve the data.
   Think: if we `dequeue` data from a queue.

   **Examples**: `strdup` creates a new string, expecting the caller to free it.

- A caller *passes a pointer* into a function, but only allows the function to *borrow* the data.
   Thus the caller still owns the memory (thus is still responsible to `free` the data) after the function returns, and the function should *not* maintain any references to the data.
   Think: most of the string functions that take a string as an argument, perform some operation on it, and return expecting the caller to still `free` the string.

    **Examples**: Most other functions we've seen borrow pointers, perform operations, and then don't maintain references to them.

- A function *returns a pointer* to the function caller that enables the caller to *borrow*  the data.
    This requires a difficult constraint: the caller can access the data, but must not maintain a pointer to it after the function or API (that still owns the data) `free`s it.

    **Examples**:
    The key-value store's `get` function transfers ownership to the caller.

The memory ownership constraints are an agreement between the calling function, and a function being called.
