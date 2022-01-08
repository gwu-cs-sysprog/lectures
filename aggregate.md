---
title: "GWU CSCi 3410: Systems Programming"
author: [Gabriel Parmer]
keywords: [UNIX, POSIX, C, Systems]
subtitle: "Version 2021-12-25"
titlepage: true
titlepage-color: "1034A6"
titlepage-text-color: "FFFFFF"
titlepage-rule-color: "FFFFFF"
titlepage-rule-height: 4
toc-own-page: true
listing-no-page-break: true
---

# Intermediate C

## Objectives

- Recalling C types with a focus on pointers.
- Practice thinking about C programs as memory

## Types

### [Basic types](https://en.wikipedia.org/wiki/C_data_types)

- `char` - think "one byte".
    Same as `signed char`.
- `short int` - think "two bytes".
    Same as `short`.
- `int` - think "four bytes".
- `long int` - think "potentially larger `int`".
	Most commonly known solely as `long`.
- `float`, `double` - floating point values (not used in the class).
- `void` - the lack of a type.
    Variables *cannot* have this type.

```c
/* `void` is fine to denote "no variables/return values", ... */
int
main(void)
{
	/* ...but not fine on variables */
	void a;

	return 0;
}
```

Program output:
```
inline_exec_tmp.c: In function main:
inline_exec_tmp.c:6:7: error: variable or field a declared void
    6 |  void a;
      |       ^
make[1]: *** [Makefile:25: inline_exec_tmp] Error 1
```

- `enum` - an `int` or `short int` with some "named" values.

```c
#include <stdio.h>

enum weekdays {
	MON, TUES, WED, THURS, FRI
};

int
main(void)
{
	printf("MON = %d, TUES = %d, ..., FRI = %d\n", MON, TUES, FRI);

	return 0;
}
```

Program output:
```
MON = 0, TUES = 1, ..., FRI = 4
```

You can choose the values explicitly (e.g. `enum grades { MON = 1, TUES = 2, WED = 3, THURS = 4, FRI = 5};`).

**Modifiers**

- `unsigned` - variables that cannot be negative. Given that variables have a fixed bit-width, they can use the extra bit ("negative" no longer needs to be tracked) to instead represent numbers twice the size of `signed` variants.
- `signed` - signed variables.
    You don't see this modifier as much because `char`, `int`, `long` all default to `signed`.
- `long` - Used to modify another type to make it larger in some cases.
	`long int` can represent larger numbers and is synonymous with `long`.
	`long long int` (or `long long`) is an even larger value!
- `const` - an immutable value.
	We won't focus much on this modifier.

#### Examples

```c
int
main(void)
{
	char a;
	signed char a_signed;     /* same type as `a` */
	char b;                   /* values between [-128, 127] */
	unsigned char b_unsigned; /* values between [0, 256] */
	int c;
	short int c_shortint;
	short c_short;            /* same as `c_shortint` */
	long int c_longint;
	long c_long;              /* same type as `c_longint` */

	return 0;
}
```

Program output:
```
```

You might see all of these, but the common primitives, and their sizes:

```c
#include <stdio.h>

/* Commonly used types that you practically see in a lot of C */
int
main(void)
{
	char c;
	unsigned char uc;
	short s;
	unsigned short us;
	int i;
	unsigned int ui;
	long l;
	unsigned long ul;

	printf("char:\t%ld\nshort:\t%ld\nint:\t%ld\nlong:\t%ld\n",
		sizeof(c), sizeof(s), sizeof(i), sizeof(l));

	return 0;
}
```

Program output:
```
char:	1
short:	2
int:	4
long:	8
```

### Common POSIX types and values

- `stddef.h`^[Which is provided by `gcc` and can be found in `/usr/include/gcc/x86_64-linux-gnu/9/include/stddef.h`.]:

    - `size_t`, `usize_t`, `ssize_t` - types for variables that correspond to sizes.
	    These include the size of the memory request to `malloc`, the return value from `sizeof`, and the arguments and return values from `read`/`write`/...
		`ssize_t` is signed (allows negative values), while the others are unsigned.
	- `NULL` - is just `#define NULL ((void *)0)`

- `limits.h`^[In `/usr/include/limits.h`.]:

	- `INT_MAX`, `INT_MIN`, `UINT_MAX` - maximum and minimum values for a signed integer, and the maximum value for an unsigned integer.
	- `LONG_MAX`, `LONG_MIN`, `ULONG_MAX` - minimum and maximum numerical values for `long`s and `unsigned long`s.
	- Same for `short ints` (`SHRT_MAX`, etc...) and `char`s (`CHAR_MAX`, etc...).

### Format Strings

Many standard library calls take "format strings".
You've seen these in `printf`.
The following format specifiers should be used:

- `%d` - `int`
- `%ld` - `long int`
- `%u` - `unsigned int`
- `%c` - `char`
- `%x` - `unsigned int` printed as hexadecimal
- `%lx` - `long unsigned int` printed as hexadecimal
- `%p` - prints out any pointer value, `void *`
- `%s` - prints out a string, `char *`

Format strings are also used in `scanf` functions to read and parse input.

You can control the spacing of the printouts using `%NX` where `N` is the number of characters you want printed out (as spaces), and `X` is the format specifier above.
For example, `"%10ld"`would print a long integer in a 10 character slot.
Adding `\n` and `\t` add in the newlines and the tabs.
If you need to print out a "\\", use `\\`.

#### Example

```c
#include <stdio.h>
#include <limits.h>

int
main(void)
{
	printf("Integers: %d, %ld, %u, %c\n"
		   "Hex and pointers: %lx, %p\n"
		   "Strings: %s\n",
		   INT_MAX, LONG_MAX, UINT_MAX, '*',
		   LONG_MAX, &main,
		   "hello world");

	return 0;
}
```

Program output:
```
Integers: 2147483647, 9223372036854775807, 4294967295, *
Hex and pointers: 7fffffffffffffff, 0x560487887149
Strings: hello world
```

### Compound types

- `struct` - A collection of different values.
    Example: objects with many different fields.
- `union` - *One* of a set of values.
    Example: what if you want to represent data for one food item among others.

Unions are not very common, but are sometimes useful.
The size of a `union` is the *maximum* size of its fields.
In contrast, the `struct` is the *sum* of the sizes of each of its fields.

#### Example

```c
#include <stdio.h>

struct hamburger {
	int num_burgers;
	int cheese;
	int num_patties;
};

union food {
	int num_eggs;
	struct hamburger burger;
};

/* Same contents as the union. */
struct all_food {
	int num_eggs;
	struct hamburger burger;
};

int
main(void)
{
	union food f_eggs, f_burger;

	f_eggs.num_eggs = 10; /* now I shouldn't access `.burger` in `f_eggs` */
	/* This is just syntax for structure initialization. */
	f_burger.burger = (struct hamburger) {
		.num_burgers = 5,
		.cheese      = 1,
		.num_patties = 1
	};                    /* now shouldn't access `.num_eggs` in `f_burger` */

	printf("Size of union:  %ld\nSize of struct: %ld\n",
		   sizeof(union food), sizeof(struct all_food));

	return 0;
}
```

Program output:
```
Size of union:  12
Size of struct: 16
```

Note: The structure initialization syntax in this example is simply a shorthand.
The `struct hamburger` initialization above is equivalent to:

```
f_burger.burger.num_burgers = 5;
f_burger.burger.cheese      = 1;
f_burger.burger.num_patties = 1;
```

Though since there are so many `.`s, this is a little confusing.
We'd typically want to simply as:

```
struct hamburger *h = &f_burger.burger;
h->num_burgers = 5;
h->cheese      = 1;
h->num_patties = 1;
```

More on `->` in the next section.

### Pointers & Arrays

Variables can have be pointer types (e.g. `int *a`).
Variables with pointer types are pointers and hold an *address* to the a variable of the type to which they point, or to `NULL`.
`NULL` denotes that the pointer is not valid.
You want to imagine that variables hold data, for example `int a = 6`:

```
a---+
| 6 |
+---+
```

A pointer, `int *b = &a` should be imagined as a pointer:

```
b ---> a---+
       | 6 |
       +---+
```

Note that the "address of", `&` operator takes a variable and returns its address.
If you print out the pointer, `printf("%p", b)`, you'll get the address (i.e. the arrow)
To *follow the arrow*, you must *dereference* the pointer: `*b == 6`.

```c
#include <stdio.h>

int value;

void
foo(int *ptr) /* here, the `*` is part of the type "int *", i.e. a pointer to an `int` */
{
	/* here, the `*` is the dereference operation, and gives us the value that is pointed to */
	*ptr = 1;
}

int
main(void)
{
	printf("value is initialized to %d\n", value);

	foo(&value); /* `&` gives us the address of the variable `value` */
	printf("value updated to %d\n", value);

	return value - 1;
}
```

Program output:
```
value is initialized to 0
value updated to 1
```

Pointers are *necessary* as they enable us to build *linked* data-structures (linked-lists, binary trees, etc...).
Languages such as Java assume that *every single* object variable is a pointer, and since *all object variables* are pointers, they don't need special syntax for them.

Arrays are simple contiguous data items, all of the same type.
`int a[4] = {6, 7, 8, 9}` should be imagined as:

```
a ---> +---+---+---+---+
       | 6 | 7 | 8 | 9 |
       +---+---+---+---+
```

When you access an array item, `a[2] == 8`, C is really treating `a` as a pointer, doing pointer arithmetic, and dereferences to find offset `2`.

```c
#include <stdio.h>

int main(void) {
	int a[] = {6, 7, 8, 9};
	int n = 1;

	printf("0th index: %p == %p; %d == %d\n", a,     &a[0], *a,       a[0]);
	printf("nth index: %p == %p; %d == %d\n", a + n, &a[n], *(a + n), a[n]);

	return 0;
}
```

Program output:
```
0th index: 0x7fff5f51e100 == 0x7fff5f51e100; 6 == 6
nth index: 0x7fff5f51e104 == 0x7fff5f51e104; 7 == 7
```

Making this a little more clear, lets understand how C accesses the `n`th item.
Lets make a pointer `int *p = a + 1` (we'll just simply and assume that `n == 1` here), we should have this:

```
p ---------+
           |
		   V
a ---> +---+---+---+---+
       | 6 | 7 | 8 | 9 |
       +---+---+---+---+
```

Thus if we dereference `p`, we access the `1`st index, and access the value `7`.

```c
#include <stdio.h>

int
main(void)
{
	int a[] = {6, 7, 8, 9};
	/* same thing as the previous example, just making the pointer explicit */
	int *p = a + 1;

	printf("nth index: %p == %p; %d == %d\n", p, &a[1], *p, a[1]);

	return 0;
}
```

Program output:
```
nth index: 0x7ffe47d222c4 == 0x7ffe47d222c4; 7 == 7
```

We can see that *pointer arithmetic* (i.e. doing addition/subtraction on pointers) does the same thing as array indexing plus a dereference.
That is, `*(a + 1) == a[1]`.
For the most part, arrays and pointers can be viewed as very similar, with only a few exceptions^[Arrays differ from pointers as 1. the `sizeof` operator on an array variable gives the total size of the array (including all elements) whereas for pointers, it returns the size of the pointer (i.e. 8 bytes on a 64 bit architecture); 2. `&` on an array returns the address of the first item in the array whereas for pointers, it returns the address of the pointer; and 3. assignments to pointers end up initializing the pointer to an address, or doing math on the pointer whereas for arrays, initialization will initialize items in the array, and math/assignment after initialization are not allowed.].

Pointer arithmetic should generally be avoided in favor of using the array syntax.
One complication is that it does not fit our intuition for addition:

```c
#include <stdio.h>

int
main(void)
{
	int a[] = {6, 7, 8, 9};
	char b[] = {'a', 'b', 'c', 'd'};
	/* Calculation: How big is the array? How big is each item? The division is the number of items. */
	int num_items = sizeof(a) / sizeof(a[0]);
	int i;

	for (i = 0; i < num_items; i++) {
		printf("idx %d @ %p & %p\n", i, a + i, b + i);
	}

	return 0;
}
```

Program output:
```
idx 0 @ 0x7fffee9091e0 & 0x7fffee9091f4
idx 1 @ 0x7fffee9091e4 & 0x7fffee9091f5
idx 2 @ 0x7fffee9091e8 & 0x7fffee9091f6
idx 3 @ 0x7fffee9091ec & 0x7fffee9091f7
```

Note that the pointer for the integer array (`a`) is being incremented by 4, while the character array (`b`) by 1.
Focusing on the key part:

```
idx 0 @ ...0 & ...4
idx 1 @ ...4 & ...5
idx 2 @ ...8 & ...6
idx 3 @ ...c & ...7
           ^      ^
           |      |
Adds 4 ----+      |
Adds 1 -----------+
```

Thus, pointer arithmetic depends on the *size of the types within the array*.
There are types when one wants to iterate through each byte of an array, even if the array contains larger values such as integers.
For example, the `memset` and `memcmp` functions set each byte in an range of memory, and byte-wise compare two ranges of memory.
In such cases, *casts* can be used to manipulate the pointer type (e.g. `(char *)a` enables `a` to not be referenced with pointer arithmetic that iterates through bytes).

#### Examples

```c
#include <stdio.h>

/* a simple linked list */
struct student {
	char *name;
	struct student *next;
};

struct student students[] = {
	{.name = "Penny", .next = &students[1]}, /* or `students + 1` */
	{.name = "Gabe",  .next = NULL}
};
struct student *head = students;
/*
 * head --> students+------+
 *          | Penny | Gabe |
 *          | next  | next |
 *          +-|-----+---|--+
 *            |     ^   +----->NULL
 *            |     |
 *            +-----+
 */
int
main(void)
{
	struct student *i;

	for (i = head; i != NULL; i = i->next) {
		printf("%s\n", i->name);
	}

	return 0;
}

```

Program output:
```
Penny
Gabe
```

#### Pointer Types

Generally, if you want to treat a pointer type as another, you need to use a cast.
You rarely want to do this (see the `memset` example below to see an example where you might want it).
However, there is a need in C to have a "generic pointer type" that can be implicitly cast into any other pointer type.
To get a sense of why this is, two simple examples:

> 1. What should the type of `NULL` be?

`NULL` is used as a valid value for *any* pointer (of any type), thus `NULL` must have a generic type that can be used in the code as a value for any pointer.
Thus, the type of `NULL` must be `void *`.

> 2. `malloc` returns a pointer to newly-allocated memory.
>     What should the type of the return value be?

C solves this with the `void *` pointer type.
Recall that `void` is not a valid type for a variable, but a `void *` is different.
It is a "generic pointer that cannot be dereferenced*.
Note that dereferencing a `void *` pointer shouldn't work as `void ` is not a valid variable type (e.g. `void *a; *a = 10;` doesn't make much sense because `*a` is type `void`).

```c
#include <malloc.h>

int
main(void)
{
	int *intptr = malloc(sizeof(int)); /* malloc returns `void *`! */

	*intptr = 0;

	return *intptr;
}
```

Program output:
```
```

Data-structures often aim to store data of any type (think: a linked list of anything).
Thus, in C, you often see `void *`s to reference the data they store.

#### Relationship between Pointers, Arrays, and Arrows

Indexing into arrays (`a[b]`) and arrows (`a->b`) are redundant syntactic features, but they are very convenient.

- `&a[b]` is equivalent to `a + b` where `a` is a pointer and `b` is an index.
- `a[b]` is equivalent to `*(a + b)` where `a` is a pointer and `b` is an index.
- `a->b` is equivalent to `(*a).b` where `a` is a pointer to a variable with a structure type that has `b` as a field.

Generally, you should always try and stick to the array and arrow syntax were possible, as it makes your intention much more clear when coding than the pointer arithmetic and dereferences.

## Example: C is a Thin Language Layer on Top of Memory

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
	printf("Addresses:\na   @ %p\nb   @ %p\nc   @ %p\nend @ %p\n&end - &a = %ld\n",
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
//	print_values();

	return 0;
}

void
print_values(void)
{
	printf("a     = %d\nb     = %d\nc.c_a = %ld\nc.c_b = %ld\nc.c_c = %p\n", a, b, c.c_a, c.c_b, c.c_c);
}
```

Program output:
```
Addresses:
a   @ 0x55b6746b4010
b   @ 0x55b6746b4014
c   @ 0x55b6746b4020
end @ 0x55b6746b4039
&end - &a = 41

Initial values:
a     = 1
b     = 2
c.c_a = 3
c.c_b = 0
c.c_c = 0x55b6746b4014

Print out the variables as raw memory


Post-`memset` values:
```

### Takeaways

1. Each variable in C (including fields in structs) want to be aligned on a boundary equal to the variable's type's size.
    This means that a variable (`b`) with an integer type (`sizeof(int) == 4`) should always have an address that is a multiple of its size (`&b % sizeof(b) == 0`, so an `int`'s address is always divisible by `4`, a `long`'s by `8`).
2. The operation to figure out the size of all the variables, `&end - &a`, is *crazy*.
    We're used to performing math operations values on things of the same type, but not on *pointers*.
	This is only possible because C sees the variables are chunks of memory that happen to be laid out in memory, one after the other.
3. The *crazy* increases with `mem = &a`, and our iteration through `mem[i]`.
	We're able to completely ignore the types in C, and access memory directly!

**Question 5**: What would break if we changed `char a;` into `int a;`?
C doesn't let us do math on variables of *any type*.

## Memory Allocation

Dynamic memory allocations

- `void *malloc(size_t size)` - allocate `size` memory, or return `NULL`.
- `void *calloc(size_t nmemb, size_t size)` - allocate `nmemb * size` bytes, and initialize them to `0`.
- `void *realloc(void *ptr, size_t size)` - pass in a previous allocation, and *either* grow/shrink it to `size`, or allocate a new chunk of memory and copy the data in `ptr` into it.
    Return the memory of size `size`, or `NULL` on error.
- `void free(void *ptr)` - deallocate previously allocated memory (returned from any of the above).

A few things to keep in mind:

1. If you want to allocate an array, then you have to do the math yourself for the array size.
    For example, `int *arr = malloc(sizeof(int) * n);` to allocate an array of `int`s with a length of `n`.
1. `malloc` is not guaranteed to initialize its memory to `0`.
    *You* must make sure that your array gets initialized.
	It is not uncommon to do a `memset(arr, 0, sizeof(int) * n);` to set the memory `0`.
1. `calloc` is guaranteed to initialize all its allocated memory to `0`.

### Common Errors

- *Allocation error.*
    You *must* check the return value of memory allocation functions for `NULL`, and handle the error appropriately.
- *Dangling pointer.*
    If you maintain a pointer to a chunk of memory that you `free`, and then dereference that pointer, bad things can happen.
	The memory might have already been re-allocated due to another call to `malloc`, and is used for something completely different in your program.
	It is up to you as a programmer to avoid `free`ing memory until all references to it are dropped.
- *Memory leaks.*
    If you remove all references to memory, but *don't* `free` it, then the memory will *never* get freed.
	This is a memory leak.
- *Double `free`.*
    If you `free` the memory twice, bad things can happen.
	You could confuse the memory allocation logic, or you could accidentally `free` an allocation made after the first `free` was called.

`valgrind` will help you debug the last three of these issues, and later in the class, we'll develop a library to help debug the first.

## Function Pointers

It is not uncommon for programs to require some *dynamic* behavior.
A data structure might need to do different types of comparisons between its data (depending on what data is being stored).
This means that a generic data-structure (think: a hash-table) needs to somehow compare equality of the data objects it stores *without knowing what they are*!
In Java, think about the `equals` method.
Each object can define its own `equals` method to compare between objects of the same type.

In C, this is implemented using *function pointers*.
These are pointers to functions of a particular type.
They can be passed as arguments to functions, and can be dereferenced to call the function.
Let look at an example from `man 3 lsearch`:

```
void *lfind(const void *key, const void *base, size_t *nmemb, size_t size,
            int(*compar)(const void *, const void *));
```

This is a function to *find* a value in an array.
The implementation needs to understand how to compare a `key` with each of the items in the array.
For this purpose, the `compar` function pointer is the last argument.

### Typedefs

Function pointer syntax is not natural in C.
Thus, it is common to use a `typedef` to create the type name for a function pointer for a given API.
The syntax for this is perhaps *even less natural*, but only needs be done once.

```
typedef int (*compare_fn_t)(const void *, const void *);
void *lfind(const void *key, const void *base, size_t *nmemb, size_t size, compare_fn_t compar);
```

Now the function declaration is more clear, and the `_fn_t` postfix, by convention, denotes a function pointer.

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

Program output:
```
0: 4 @ 0x55596a0e2048
1: 2 @ 0x55596a0e2044
2: 0 @ (nil)
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

Program output:
```
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
	n->next = ll->head;
	n->data = data;
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
```

Program output:
```
student added to list!
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

Program output:
```
What is uninitialized global memory set to?
Integer: 0
Pointer: (nil) (as hex: 0)
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

Program output:
```
Should find 8 @ 6: 6
Should find 8 @ 6: 6 (-1 is wrong here)
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

Program output:
```
0: 194
1: 0
2: -1861672505
3: 32767
4: -1861672506
5: 32767
6: 334000701
7: 22071
8: 491464
9: 32599
10: 334000624
11: 22071
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

Program output:
```
inline_exec_tmp.c: In function bar:
inline_exec_tmp.c:10:9: warning: function returns address of local variable [-Wreturn-local-addr]
   10 |  return &a;            /* Return the address of a local variable. */
      |         ^~
Save address of local variable, and dereference it: 42
make[1]: *** [Makefile:22: inline_exec] Segmentation fault (core dumped)
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

Program output:
```
42 is the secret to everything
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

Program output:
```
hi!
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

Program output:
```
"hello world and also 42" has length 23
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

Program output:
```
zzz
*** stack smashing detected ***: terminated
make[1]: *** [Makefile:22: inline_exec] Aborted (core dumped)
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

Program output:
```
extracted 1, remaining string: " 42 the secret to everything"
extracted 42, remaining string: " the secret to everything"
42 and "secret"
Using strtok to parse through a string finding substrings separated by 'h' or 't':
[1 42 ]
[e secre]
[ ]
[o every]
[ing]
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

Program output:
```
blahblahblah
0x557517993004 == 0x557517993004 != 0x557517995011
```

The C compiler and linker are smart enough to see that if you have already used a string with a specific value (in this case `"clone"`), it will avoid allocating a copy of that string, and will just reuse the previous value.
Generally, it doesn't make much sense to look at the address of strings, and certainly you should not compare them.
You can see in this example how you must compare strings for equality using `strncmp`, and *not* to compare pointers.

## API Design and Concerns

When programming in C, you'll see quite a few APIs.
Throughout the class, we'll see quite a few APIs, most documented in `man` pages.
It takes some practice in reading `man` pages to get what you need from them.
One of the things that helps the most is to understand a few common *patterns* and *requirements* that you find these APIs, and in C programming in general.

### Return Values

Function's often need to return multiple values.
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

Program output:
```
no errors!
```

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

Program output:
```
Lets get greedy: allocate 9223372036854775807 bytes!
Error: errno value 12 and description: Cannot allocate memory
Error allocating memory: Cannot allocate memory
make[1]: *** [Makefile:22: inline_exec] Error 255
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

# Processes

Programming is hard.
When you write a program, you have to consider complex logic and the design of esoteric data-structures, all while desperately trying to avoid errors.
It is an exercise that challenges all of our logical, creative, and risk management facilities.
As such, programming is the act of methodically conquering complexity with our ability to abstract.
We walk on the knives-edge of our own abstractions where we can barely, just barely, make progress and engineer amazing systems.

Imagine if when programming and debugging, we had to consider the actions and faults of *all other* programs running in the system?
If your program crashed because one of your colleagues' program had a bug, how could you make progress?

Luckily, we stand on the abstractions of those that came before us.
A key abstraction provided by systems is that of *isolation* - that one program cannot arbitrarily interfere with the execution of another.
This isolation is a core provision of Operating Systems (OSes), and one of the core abstractions that provides it is *processes*.
At the highest-level, each process can only corrupt its own memory, and a process that goes into an infinite loop cannot prevent another process for executing.

A process has a number of properties including:

- A set of memory that is only accessible to that process that includes the code, global data, stack, and dynamically allocated memory in the *heap*.
    Different processes have *disjoint* sets of memory, thus no matter how one process alters its own memory, it won't interfere with the data-structures of another.
- A number of *descriptors* identified by integers that enable the process to access the "resources" of the surrounding system including the file system, networking, and communication with other processes.
    The OS prevents processes from accessing and changing resources they shouldn't have access to.
- A set of unique identifiers including a process identifier (`pid`).
- The "current working directory" for the process, analogous to `pwd`.
- A owning user (e.g. `gparmer`) for whom the process executes on the behalf of.
- Each process has a *parent* - the other process that created it, and that has the ability and responsibility oversee it (like a normal parent).

Throughout the class we'll uncover more and more of these, but in this lecture, we'll focus on the lifecycle of a process, and its relationship to its parent.
Processes are the abstraction that provide isolation between users, and between computations.

## History: UNIX, POSIX, and Linux

UNIX is an old system, having its origins in 1969 at Bell Labs when it was created by Ken Thompson and Dennis Ritchie^[C was also created as part of the original UNIX as a necessary tool to implement the OS!].
Time has shown it is an "oldie but goodie" -- as most popular OSes are derived from it (OSX, Linux, Android, ...).
In the OS class, you'll dive into an implementation of UNIX very close to what it was in those early years!
The [original paper](figures/unix.pdf) is striking in how uninteresting it is to most modern readers.
UNIX is so profound that it has defined what is "normal" in most OSes.

> UNIX Philosophy: A core philosophy of UNIX is that applications should not be written as huge monolithic collections of millions of lines of code, and should instead be *composed* of smaller programs, each focusing on "doing one thing well".
> Programs focus on inputting text, and outputting text.
> *Pipelines* of processes take a stream of text, and process on it, process after process to get an output.
> The final program is a composition of these processes composed to together using  pipelines.

In the early years, many different UNIX variants were competing for market-share along-side the likes of DOS, Windows, OS2, etc...
This led to differences between the UNIX variants which prevented programmers from effectively writing programs that would work across the variants.
In response to this, in the late 1980s, POSIX standardized many aspects of UNIX including command-line programs and the standard library APIs.
`man` pages are documentation for what is often part of the POSIX specification.
I'll use the term UNIX throughout the class, but often mean POSIX.
You can think of Linux as an implementation of POSIX and as a variant of UNIX.

UNIX has been taken into many different directions.
Android and iOS layer a mobile runtime on top of UNIX; OSX and Ubuntu layer modern graphics and system  management on top of UNIX, and even Windows Subsystem for Linux (WSL) enables you to run a POSIX environment inside of Windows.
In many ways, UNIX has won.
However, it has won be being adapted to different domains -- you might be a little hard-pressed looking at the APIs for some of those systems to understand that it is UNIX under the hood.
Regardless, in this class, we'll be diving into UNIX and its APIs to better understand the core technology underlying most systems we use.

## Process: Life and Death

When we construct a pipeline of processes...
```
$ ps aux | grep gparmer
```
we're executing two processes that separately run the code of the programs `ps` and `grep`.
The *shell* is the parent process that creates the `ps` and `grep` processes.
It ensures the descriptor for the *output* of the `ps` process sends its data to the descriptor in the `grep` process for its *input*.

What are the functions involved in the creation, termination, and coordination between processes?

- `fork` - Create a new, child, process from the state of the calling process.
- `getpid`, `getppid` - get the unique identifier for the process, or for its parent.
- `exit` - This is the function to terminate a process.
- `wait` - A parent awaits a child exiting.
- `exec` - Execute a new *program* using the current process^[This is a little bit like when an agent transforms from a normal citizen in the Matrix.].

Lets be a little more specific.
A *program* is an executable file.
We see them in the file system (for example, see `ls /bin`), and create them when we compile our programs.
A *process* is an executing program with the previously mentioned characteristics (disjoint memory, descriptors to resources, etc...).

First, lets look at `fork`:
```c
#include <stdio.h>
#include <unistd.h> /* fork, getpid */

int
main(void)
{
	pid_t pid; /* "process identifier" */

	printf("pre-fork:\n\t parent pid %d\n", getpid());
	pid = fork(); /* so much complexity in such a simple call! */
	printf("post-fork:\n\treturned %d,\n\tcurrent pid %d\n\tparent pid %d\n",
	       pid, getpid(), getppid());

	return 0;
}
```

Program output:
```
pre-fork:
	 parent pid 345772
post-fork:
	returned 345773,
	current pid 345772
	parent pid 345766
pre-fork:
	 parent pid 345772
post-fork:
	returned 0,
	current pid 345773
	parent pid 345772
```
We can see the `fork` creates a new process that continues running the same code, but has a separate set of memory.
A few observations:

1. In this case, you can see that the `pid` variable is different for the two processes.
    In fact, after the `fork` call, *all of memory is copied* into the new process and is disjoint from that of the parent.
2. The `getpid` function returns the current process' process identifier.
3. `fork` returns *two values* - one in the parent in which it returns the process id of the newly created child, and one in the child in which it returns `0`.
    This return value is one of the main indicators of if the current process is the parent, or the child.

So much complexity for such a simple function prototype!

**`exit` and `wait`.**
In contrast to `fork` that creates a new process, `exit` is the function that used to terminate the current process.
`exit`'s integer argument is the "exit status" -- `0` is equivalent to `EXIT_SUCCESS`, and denotes a successful execution of the program, and `-1` is `EXIT_FAILURE`, and denotes execution failure.
The return value from `main` is also this exit status.
Where is the exit status used?

Parent processes have the responsibility to manage their children.
This starts with the `wait` function.
A parent that calls `wait` will "wait" for the child to `exit`.
Then, `wait` enables a parent to retrieve the exit status of the child.

The relationship between `wait` (in the parent), and `exit` or return from main (in the child) is a little hard to understand.
A figure always helps...

![The relationship between a `wait`ing parent, and an `exit`ing, or returning from `main` parent.](figures/wait.png)

**Adopting orphaned children.**
What happens if a parent exits before a child?
Who is allowed to `wait` and get the child's status?
Intuition might tell you that it is the *grandparent* but that is not the case.
Most programs are not written to understand that `wait` might return a `pid` other than ones they've created.
Instead, if a parent terminates, the `init` process -- the processes that is the ancestor of *all* processes -- *adopts* the child, and `wait`s for it.
We'll dive into `init` later in the class.

**Interpreting `exit` status.**
Despite `wait`'s status return value being a simple `int`, the *bits* of that integer mean very specific things.
See the `man` page for `wait` for details, but we can use a set of functions (really "macros") to interpret the status value:

- `WIFEXITED(status)` will return `0` if the process didn't exit (e.g. if it faulted instead), and non-`0` otherwise.
- `WEXITSTATUS(status)` will get the intuitive integer value that was passed to `exit` (or returned from `main`), but assumes that this value is only 8 bits large, max (thus has a maximum value of 256).

### Process Example

```c
#include <unistd.h> /* fork, getpid */
#include <wait.h>   /* wait */
#include <stdlib.h> /* exit */
#include <stdio.h>

int
main(void)
{
	pid_t pid, child_pid;
	int i, status;

	/* Make four children. */
	for (i = 0; i < 4; i++) {
		pid = fork();
		if (pid == 0) { /* Are we the child? */
			printf("Child %d exiting with %d\n", getpid(), -i);
			if (i % 2 == 0) exit(-i);   /* terminate immediately! */
			else            return -i;  /* also, terminate */
		}
	}

	/*
	 * We are the parent! Loop until wait returns `-1`, thus there are no more children. Note
	 * that this strange syntax says "take the return value from `wait`, put it into the variable
	 * `child_pid`, then compare that variable to `-1`".
	 */
	while ((child_pid = wait(&status)) != -1) { /* no more children when `wait` returns `-1`! */
		/* wait treats the `status` as the second return value */
		if (WIFEXITED(status)) {
			/* Question: why the `(char)` cast? */
			printf("Child %d exited with exit status %d\n", child_pid, (char)WEXITSTATUS(status));
		}
	}

	return 0;
}
```

Program output:
```
Child 345789 exiting with 0
Child 345790 exiting with -1
Child 345791 exiting with -2
Child 345792 exiting with -3
Child 345789 exited with exit status 0
Child 345790 exited with exit status -1
Child 345791 exited with exit status -2
Child 345792 exited with exit status -3
```

### An Aside: Concurrency

The output of this program can actually vary, somewhat arbitrarily, in the following way:
any of the output lines can be reordered with respect to any other, *except* that the parent's print out for a specific child must come after it.

For example, all of the following are possible:
```
Child 4579 exiting with 0
Child 4580 exiting with -1
Child 4581 exiting with -2
Child 4582 exiting with -3
Child 4579 exited with exit status 0
Child 4580 exited with exit status -1
Child 4581 exited with exit status -2
Child 4582 exited with exit status -3
```

```
Child 4579 exiting with 0
Child 4579 exited with exit status 0
Child 4580 exiting with -1
Child 4580 exited with exit status -1
Child 4581 exiting with -2
Child 4581 exited with exit status -2
Child 4582 exiting with -3
Child 4582 exited with exit status -3
```

```
Child 4582 exiting with -3
Child 4582 exited with exit status -3
Child 4581 exiting with -2
Child 4581 exited with exit status -2
Child 4580 exiting with -1
Child 4580 exited with exit status -1
Child 4579 exiting with 0
Child 4579 exited with exit status 0
```

This *non-determinism* is a product of the *isolation* that is provided by processes.
The OS switches back and forth between processes frequently (up to thousands of time per second!) so that if one goes into an infinite loop, others will still make progress.
But this also means that the OS can choose to run any of the processes that are trying to execute at any point in time!
We cannot predict the order of execution, completion, or `wait` notification.
This non-deterministic execution is called *concurrency*.
You'll want to keep this in mind as you continue to learn the process APIs, and when we talk about IPC, later.

### Taking Actions on `exit`

Lets dive into `exit` a little bit.

``` c
#include <stdlib.h>

int
main(void)
{
	exit(EXIT_SUCCESS);
}
```
Believe it or not, this is *identical* to
``` c
#include <stdlib.h>

int
main(void)
{
	return EXIT_SUCCESS;
}
```
...because when `main` returns, it calls `exit`.

> **Investigating `main` return → `exit` via `gdb`.**
> You can see this by diving into `gdb -tui`, breakpointing before the return (e.g. `b 5`), and single-stepping through the program.
> You'll want to `layout asm` to drop into "assembly mode", and single step through the assembly and if you want to step through it instruction at a time use `stepi` or `si`.
> You can see that it ends up calling `__GI_exit`.
> `__GI_*` functions are glibc internal functions, so we see that `libc` is actually calling `main`, and when it returns, it is then going through its logic for `exit`.

Though `exit` is a relatively simple API, it has a few interesting features.

- `on_exit` which takes a function pointer to be called upon returning from `main` or calling `exit`.
    The function receives the status, and an argument pass to `on_exit`.
	So we can see that calling `exit` does *not* immediately terminate the process!
- `atexit` takes a simpler function pointer (that doesn't receive any arguments) to call at exit.
- `_exit` terminates a process *without calling any of the function pointers* set up by `on_exit` nor `atexit`.

```c
#include <stdlib.h>
#include <stdio.h>

int value = 0;

void
destructor(int status, void *data)
{
	int *val = data;

	printf("post-exit: status %d, data %d\n", status, *val);
}

int
main(void) {
	on_exit(destructor, &value); /* `value` will get passed to the function */

	value = 1;

	printf("pre-exit\n");

	return 0;
}
```

Program output:
```
pre-exit
post-exit: status 0, data 1
```

### `fork` Exercises

1. How many processes (not including the initial one) are created in the following?

	```c
	#include <unistd.h>

	int
	main(void)
	{
		fork();
		fork();

		return 0;
	}
	```

2. What are the *possible outputs* for:

	``` c
	#include <unistd.h>
	#include <stdio.h>
	#include <sys/wait.h>

	int
	main(void)
	{
		pid_t parent = getpid();
		pid_t child;

		child = fork();
		if (getpid() == parent) {
			printf("p");
			wait(NULL); /* why is `NULL` OK here? */
			printf("w");
		} else {
			printf("c");
		}

		return 0;
	}
	```

3. Implement `forkonacci`!
    This will solve the `n`th [fibonacci number](https://en.wikipedia.org/wiki/Fibonacci_number), using `fork` for the recursion, and a combination of `exit` to "return" and `wait` to retrieve the returned value, instead of function calls.
	The normal recursive implementation:

	``` c
	unsigned int
	fibonacci(unsigned int n)
	{
	    if (n == 0 || n == 1) return n;

 		return fibonacci(n - 1) + fibonacci(n - 2);
	}
	```

	This will work decently for `n <= 12`, but not for `n > 12`.
	Why^[Hint: make sure to read the `man` pages for `exit(1)`]?

## Executing a Program

Recall, a *program* is an executable that you store in your file-system.
These are the output of your compilation, the "binaries".
So `fork` lets us make a new process from an existing process, both of which are executing within a single program.
But how do we execute a *new* program?

The `exec` family of system calls will do the following:

- Stop executing in the current process.
- Reclaim all memory within the current process.
- Load the target program into the process.
- Start executing in the target program (i.e. starting normally, resulting in `main` execution).

The core insight is that the *same process* continues execution; it simply continues execution in the new program.
This is a little unnatural, but has a few handy side-effects:

- The execution of the new program inherits the process identifier (`pid_t`) and the parent/child relationships of the process.
- Comparably, the *descriptors* are inherited into the new program's execution.
- The environment variables (see section below) pass to the new program's execution.
- Many other process properties are comparably inherited.
    With the exception of the process memory, you can assume, by default, that process properties are inherited across an `exec`.

### `exec` APIs

There are a few ways to execute a program:

- `execl`
- `execlp`
- `execle`
- `execv`
- `execvp`

The naming scheme is quite annoying and hard to remember, but the `man` page has a decent summary.
The trailing characters correspond to specific operations that differ in how the command-line arguments are passed to the `main` (`l` means pass the arguments to this `exec` call to the program,  while `v` means pass the arguments as an array of the arguments into the `exec` call), how the program's path is specified (by default, an "absolute path" starting with `/` must be used, but in a `v` variant, the binary is looked up using comparable logic to your shell), and how environment variables are passed to the program.
For now, we'll simply use `execvp`, and cover the rest in subsequent sections.

```c
#include <stdio.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	int status;
	pid_t pid;

	if (fork() == 0) { /* child */
		char *args[] = {"./03/hw.bin", NULL}; /* requires a NULL-terminated array of arguments */

		/* here we use a relative path (one that starts with `.`), so we must use the `p` postfix */
		if (execvp("./03/hw.bin", args)) {
			/*
			 * This should really *not return* in the nromal case. This memory context
			 * goes away when we `exec`, and is replaced by the new program, thus simply
			 * removing this code. Thus, this will only continue executing if an error
			 * occured.
			 */
			perror("exec");

			return EXIT_FAILURE;
		}
	}

	pid = wait(&status);
	assert(pid != -1);

	printf("Parent: status %d\n", WEXITSTATUS(status));

	return 0;
}
```

Program output:
```
hello world!
Parent: status 43
```

## Command Line Arguments

I think that we likely have a decent intuition about what the command-line arguments are:

```
$ ls /bin /sbin
```

The `ls` program takes two arguments, `/bin` and `/sbin`.
How does `ls` access those arguments?
Lets look at a *chain of programs* that `exec` each other.
The first program (that you see here) is called `inline_exec_tmp`, and the programs `03/args?.c` are subsequently executed.
```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char *prog = "./03/args1.bin";

int
main(int argc, char *argv[])
{
	char *args[] = {prog, "hello", "world", NULL};

	if (argc != 1) return EXIT_FAILURE;

	printf("First program, arg 1: %s\n", argv[0]);
	fflush(stdout);

	/* lets execute args1 with some arguments! */
	if (execvp(prog, args)) {
		perror("exec");
		return EXIT_FAILURE;
	}

	return 0;
}
```

Program output:
```
First program, arg 1: ./inline_exec_tmp
Inside ./03/args2.bin
arg 0: ./03/args2.bin
arg 1: hello
arg 2: world
```

So we see the following.

- It is *convention* that the first argument to a program is always the program itself.
    The shell will *always* ensure that this is the case (NB: the shell `exec`s your programs).
- The rest of the arguments are passed as separate entries in the array of arguments.
- The `v` variants of `exec` require the `NULL` termination of the argument array, something that is easy to mess up!

Parsing through the command-line arguments can be a little annoying, and `getopt` can help.

## Environment Variables

Environment variables are UNIX's means of providing configuration information to any process that might want it.
They are a key-value store^[Yes, yes, yes, I know this is getting redundant. Key-value stores are *everywhere*!] that maps an environment variable to its value (both are strings).

Environment variables are used to make configuration information accessible to programs.
They are used instead of command-line arguments when:

- You don't want the user worrying about passing the variable to a program.
    For example, you don't want the user to have to pass their username along to every program as an argument.
- You don't know which programs are going to use the configuration data, but *any* of them *could*.
    You don't want to pass a bunch of command-line variables into each program, and expect them to pass them along to each child process.

Example common environment variables include:

- `PATH` - a `:`-separated list of file system paths to use to look for programs when attempt to execute a program.
- `HOME` - the current user's home directory (e.g. `/home/gparmer`).
- `USER` - the username (e.g. `gparmer`).
- `TEMP` - a directory that you can use to store temporary files.

Many programs setup and use their own environment variables.
Note that environment variables are pretty pervasively used -- simple libraries exist to access them from `python`, `node.js`, `rust`, `java`, etc...

You can easily access environment variables from the command line:

```
$ echo $HOME
/home/gparmer
$ export BESTPUP=penny
$ echo $BESTPUP
penny
```

Any program executed from that shell, will be able to access the "`BESTPUP`" environment variable.
The `env` command dumps out all current environment variables.

### Environment Variable APIs

So how do we access the environment variable key-value store in C?
The core functions for working with environment variables include:

- `getenv` - Get an environment variable's value.
- `setenv` - Set one of environment variable's value (used by the shell to set up children's variables).
- `clearenv` - Reset the entire environment.
- `environ` array - This is the array of environment variables you'll see in the `man` pages.
    You don't want to access/modify this directly, but you can imagine it is used to back all of the previous calls.

```c
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int
main(int argc, char *argv[])
{
	char *u = getenv("USER");
	char *h = getenv("HOME");

	assert(u && h);

	printf("I am %s, and I live in %s\n", u, h);

	return 0;
}
```

Program output:
```
I am gparmer, and I live in /home/gparmer
```

You can see all of the environmental variables available by default with:

```
$ env
SHELL=/bin/bash
DESKTOP_SESSION=ubuntu
EDITOR=emacs -nw
PWD=/home/gparmer/repos/gwu-cs-sysprog/22/lectures
LOGNAME=gparmer
HOME=/home/gparmer
USERNAME=gparmer
USER=gparmer
PATH=/home/gparmer/.local/bin::/home/gparmer/.cargo/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin:/usr/racket/bin/
DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus
...
```


```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int
main(int argc, char *argv[])
{
	char *u = getenv("USER");

	assert(u);
	printf("user: %s\n", u);
	fflush(stdout);

	if (setenv("USER", "penny", 1)) {
		perror("attempting setenv");
		exit(EXIT_FAILURE);
	}

	if (fork() == 0) {
		char *u = getenv("USER");
		char *args[] = { "./03/envtest.bin", "USER", NULL };

	    /* environment is inherited across *both* `fork` and `exec`! */
		printf("user (forked child): %s\n", u);
		fflush(stdout);
		if (execvp("./03/envtest.bin", args)) {
			perror("exec");

			return EXIT_FAILURE;
		}
	}

	return 0;
}
```

Program output:
```
user: gparmer
user (forked child): penny
Environment variable USER -> penny
```

`03/envtest.c` is
``` c
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int
main(int argc, char *argv[])
{
	char *e = "NOARG";
	char *v = "NOVAL";

	if (argc == 2) {
		e = argv[1];
		v = getenv(e);
		if (!v) {
			v = "";
		}
	}

	printf("Environment variable %s -> %s\n", e, v);

	return 0;
}
```

**Question.**
How do you think that the shell-based support for environment variables (`export BESTPUP=penny`) is implemented?
Specifically, if we do the following...

```
$ export BESTPUP=penny
$ ./03/envtest.bin BESTPUP
Environment variable BESTPUP -> penny
```

...which process is using which APIs?
Put another way, how is `export` implemented?

## Representations of Processes

We've seen how to create and manage processes and how to execute programs.
Amazingly, modern systems have pretty spectacular facilities for *introspection* into executing processes.
Introspection facilities generally let you dive into something as it runs.
The most immediate example of this is `gdb` or any debugger that let you dive into an implementation.
Looking at `pause.c`:

``` c
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
```

Program output:

```
pid 9310
global (RO):    0x55be6d40b008
global:         0x55be6d40d014
stack:          0x7ffc7abafc7c
heap:           0x55be6da162a0
function:       0x55be6d40a1c9
```

Lets take the process identifier, and *dive into the process!*
The "`proc` filesystem" in Linux is a part of the file system devoted to representing processes.
There is a subdirectly in it for each process in the system.
Lets check out process `9310`.

```
$ cd /proc/9310/
$ ls
arch_status  cgroup      coredump_filter  exe      io         maps       mountstats  oom_adj        patch_state  sched      smaps         statm    timers
attr         clear_refs  cpuset           fd       limits     mem        net         oom_score      personality  schedstat  smaps_rollup  status   timerslack_ns
autogroup    cmdline     cwd              fdinfo   loginuid   mountinfo  ns          oom_score_adj  projid_map   sessionid  stack         syscall  uid_map
auxv         comm        environ          gid_map  map_files  mounts     numa_maps   pagemap        root         setgroups  stat          task     wchan
$ cat maps
55be6d409000-55be6d40a000 r--p 00000000 08:02 1315893                    /home/ycombinator/repos/gwu-cs-sysprog/22/lectures/03/pause.bin
55be6d40a000-55be6d40b000 r-xp 00001000 08:02 1315893                    /home/ycombinator/repos/gwu-cs-sysprog/22/lectures/03/pause.bin
55be6d40b000-55be6d40c000 r--p 00002000 08:02 1315893                    /home/ycombinator/repos/gwu-cs-sysprog/22/lectures/03/pause.bin
55be6d40c000-55be6d40d000 r--p 00002000 08:02 1315893                    /home/ycombinator/repos/gwu-cs-sysprog/22/lectures/03/pause.bin
55be6d40d000-55be6d40e000 rw-p 00003000 08:02 1315893                    /home/ycombinator/repos/gwu-cs-sysprog/22/lectures/03/pause.bin
55be6da16000-55be6da37000 rw-p 00000000 00:00 0                          [heap]
7ff4a127f000-7ff4a12a4000 r--p 00000000 08:02 11797912                   /lib/x86_64-linux-gnu/libc-2.31.so
7ff4a12a4000-7ff4a141c000 r-xp 00025000 08:02 11797912                   /lib/x86_64-linux-gnu/libc-2.31.so
7ff4a141c000-7ff4a1466000 r--p 0019d000 08:02 11797912                   /lib/x86_64-linux-gnu/libc-2.31.so
7ff4a1466000-7ff4a1467000 ---p 001e7000 08:02 11797912                   /lib/x86_64-linux-gnu/libc-2.31.so
7ff4a1467000-7ff4a146a000 r--p 001e7000 08:02 11797912                   /lib/x86_64-linux-gnu/libc-2.31.so
7ff4a146a000-7ff4a146d000 rw-p 001ea000 08:02 11797912                   /lib/x86_64-linux-gnu/libc-2.31.so
7ff4a146d000-7ff4a1473000 rw-p 00000000 00:00 0
7ff4a1495000-7ff4a1496000 r--p 00000000 08:02 11797896                   /lib/x86_64-linux-gnu/ld-2.31.so
7ff4a1496000-7ff4a14b9000 r-xp 00001000 08:02 11797896                   /lib/x86_64-linux-gnu/ld-2.31.so
7ff4a14b9000-7ff4a14c1000 r--p 00024000 08:02 11797896                   /lib/x86_64-linux-gnu/ld-2.31.so
7ff4a14c2000-7ff4a14c3000 r--p 0002c000 08:02 11797896                   /lib/x86_64-linux-gnu/ld-2.31.so
7ff4a14c3000-7ff4a14c4000 rw-p 0002d000 08:02 11797896                   /lib/x86_64-linux-gnu/ld-2.31.so
7ff4a14c4000-7ff4a14c5000 rw-p 00000000 00:00 0
7ffc7ab90000-7ffc7abb1000 rw-p 00000000 00:00 0                          [stack]
7ffc7abe1000-7ffc7abe4000 r--p 00000000 00:00 0                          [vvar]
7ffc7abe4000-7ffc7abe5000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
```

There's a **lot** going on here.
The most important parts of the *ranges* of addresses on the left that tell us where we can find the `pause.bin` program's code, read-only global data, global read-writable data, heap, and stack!
We also see that the number of maps in a very simple process is surprisingly large.
Let me manually focus in on a few parts of this.

```
...
55be6d40a000-55be6d40b000 r-xp ... /03/pause.bin <-- &main      (0x55be6d40a1c9)
55be6d40b000-55be6d40c000 r--p ... /03/pause.bin <-- &global_ro (0x55be6d40b008)
...
55be6d40d000-55be6d40e000 rw-p ... /03/pause.bin <-- &global    (0x55be6d40d014)
...
55be6da16000-55be6da37000 rw-p ... [heap]        <-- heap       (0x55be6da162a0)
...
7ff4a14c4000-7ff4a14c5000 rw-p ...
7ffc7ab90000-7ffc7abb1000 rw-p ... [stack]       <-- stack      (0x7ffc7abafc7c)
...
```

We can see that each of the variables we're accessing in the program are at addresses that correspond to the *ranges* of memory in the `maps`.
Even more, the `/proc/9310/mem` file contains *the actual memory for the process*!
This is really amazing: we can watch the memory, even as it changes, as a process actually executes.
This is how debuggers work!

As you can see, *processes are data* too!

# Programming Process Resources

We've discussed the APIs for process manipulation and program execution, but now we're going to start discussing how processes can manipulate the resources it has access to.
Two of the key UNIX mechanisms that processes must use effectively are

- the ability manipulate and use their *descriptors* -- often called "file descriptors" though they are more general than only interfacing with files, and
- controlling and leveraging the exceptional control flow provided by *signals*.

## Process Descriptors and Pipes

Each process has a set of *descriptors* that are each associated with some *resource* in the system.
Each descriptor is an integer that is passed into a family of APIs that perform operations on their corresponding resources.
For example, most processes have at least a set of three descriptors that are each attached to a resource:

- `STDIN_FILENO` = `0` (defined in `unistd.h`) - This is the *standard input*, or the main way the process gets input from the system.
    As such, the resource is often the terminal if the user directly provides input, or sometimes the output of a previous stage in a command-line pipeline.
- `STDOUT_FILENO` = `1` - This is the *standard output*, or the main way the process sends its output to the system.
    When we call `printf`, it will, by default, output using this descriptor.
- `STDERR_FILENO` = `2` - This is the *standard error*, or the main way that the process outputs error messages.
    `perror` (and other error-centric APIs) output to the standard error.

Each of these descriptors is associated with a potentially infinite sequence of bytes, or *channel*^["Channel" is a term that is heavily overloaded, but I'll inherit the general term from the [glibc documentation](https://www.gnu.org/software/libc/manual/html_node/Stream_002fDescriptor-Precautions.html).].
When we type at the shell, we're providing a channel of data that is sent to the standard input of the active process.
When a process prints, it sends a sequence of characters to the standard output, and because programs can loop, that stream can be infinite!
Channels, however, *can* terminate if they run out of data.
This could happen, for example, if a channel reading through a file reaches the end of the file, or if the user hits `cntl-d` on their terminal to signify "I don't have any more data".

### File Descriptor Operations

File descriptors are analogous to pointers.
The descriptors effectively *point to* channel resources.
Some of the core operations on file descriptors include:

- `read` -
    Pull a sequence of bytes from the channel into a buffer in memory.
	This is one of the core means of reading data from files, pipes, and other resources.
- `write` -
	Send a sequence of bytes from a buffer in memory into a channel.
	At its core, even `printf` ends up being a `write` to the standard out descriptor.
- `dup`/`dup2` -
    Take a file descriptor as an argument, and create a new descriptor that is a *duplicate* of the existing descriptor.
    Note, this does not duplicate the channel itself, just the descriptor.
	The analogy is that `dup` just copies the pointer, not the resource it points to.
- `close` -
    Deallocate a file descriptor.
	This is like removing a pointer -- it doesn't necessarily remove the backing channel, for example, if the descriptor was previously `dup`ed.

Lets see some of these in action.

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

int
main(void)
{
	char *hw = "hello world\n";
	char output[256];
	int fd;
	ssize_t amnt; /* signed size */

	amnt = write(STDOUT_FILENO, hw, strlen(hw));
	if (amnt == 0) { /* maybe STDOUT writes to a file with no disk space! */
		/* this is *not* an error, so errno not set! */
		printf("Cannot write more data to channel\n");
		exit(EXIT_FAILURE);
	} else if (amnt > 0) {
		/* normally, the return value tells us how much was written */
		assert(amnt == (ssize_t)strlen(hw));
	} else { /* amnt == -1 */
		perror("Error writing to stdout");
		exit(EXIT_FAILURE);
	}

	amnt = write(STDERR_FILENO, hw, strlen(hw));
	assert(amnt >= 0);

	fd = dup(STDOUT_FILENO);
	assert(fd >= 0);

	/* We can write formatted data out to stdout manually! */
	snprintf(output, 255, "in: %d, out: %d, err: %d, new: %d\n",
	         STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, fd);
	output[255] = '\0';
	amnt = write(fd, output, strlen(output));
	/* new file descriptors are supposed to use the lowest unused descriptor! */

	/* make a descriptor available */
	close(STDIN_FILENO); /* STDIN is no longer really the input! */
	fd = dup(STDOUT_FILENO);
	printf("New descriptor @ %d\n", fd);

	return 0;
}
```

Program output:
```
hello world
hello world
in: 0, out: 1, err: 2, new: 3
New descriptor @ 0
```

You can run this, and redirect the standard error to a file to see that writing to standard error is a different operation than writing to standard output.
For example: `$ prog 2> errors.txt` will redirect file descriptor `2` (stderr) to the file.

Lets focus in a little bit on `read` and `write`.
First, it is notable that the buffer they take as an argument (along with its length) is simply an array of bytes.
It can be a string, or it could be the bytes that are part of an encoded video.
Put another way, by default, *channels are just sequences of bytes*.
It is up to our program to interpret those bytes properly.
Second, we need to understand that the return value for `read`/`write` has four main, interesting conditions:

```c
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

int
main(void)
{
	ssize_t amnt;
	char *hi = "more complicated than you'd think...";
	ssize_t hi_sz = strlen(hi);

	amnt = write(STDOUT_FILENO, hi, hi_sz);

	/* Can often mean that we are not able to write to the resource */
	if (amnt == 0) {
	    /*
		 * Keep trying to write, or give up.
		 * Common return value for `read` when a file has no more data, or a pipe is closed.
		 */
	} else if (amnt > 0 && amnt < hi_sz) {
		/*
		 * Didn't write everythign we wanted, better call write again with
		 * `hi[amnt]` and `hi_sz - amnt`
		 */
	} else { /* amnt == -1 */
		/* Could be a genuine error, but not always... */
		if (errno == EPIPE || errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
		    /* conditions we should probably handle properly */
		} else {
		    /* error in the channel! */
		}
	}

	return 0;
}
```

Program output:
```
more complicated than you'd think...
```

It is common to have a convention on how channel data is structured.
UNIX pipeline encourage channels to be plain text, so that each program can read from their standard input, do some processing that can involved filtering out data or transforming it, and send the result to the standard out.
That standard output is sent to the standard input of the next process in the pipeline.
An example in which we print out each unique program that is executing on the system:

```
$ ps aux | tr -s ' ' | cut -d ' ' -f 11 | sort | uniq
```

Each of the programs in the pipeline is not configured to print out each unique process, and we are able to compose them together in a pipeline to accomplish the goal.

### Pipes

We've seen that the descriptors `0-2` are automatically set up for our processes, and we can use/manipulate them.
But how do we *create* resources?
There are many different resources in a UNIX system, but three of the main ones:

1. `files` and other file-system objects (e.g. directories),
2. `sockets` that are used to communicate over the network, and
3. communication facilities like `pipe`s that are used to send data and coordinate between processes.

Each of these has very different APIs for creating the resources.
We'll discuss files later, and will now focus on `pipe`s.

A `pipe` (a UNIX resource, not the `|` character in a command-line pipeline) is a channel referenced by *two* different file descriptors^[We see why the name "file" in file descriptors is not very accurate. Recall that I'm using the traditional "file descriptors" to denote all of our generic descriptors.], one that we can `write` to, and one that we can `read` from.
The sequence of bytes written to the pipe will be correspondingly read out.

The `pipe` and `pipe2` functions create a pipe resource, and return the two file descriptors that reference the readable and writable ends of the pipe.
Each pipe can only hold a finite *capacity*.
Thus if a process tries to write more than that capacity into a pipe, it won't complete that `write` until data is `read` from the pipe.

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/param.h> /* MIN */

/* Question: What happens when you increase `WRITE_CHUNK` (always <= sizeof(from))?? */
#define WRITE_CHUNK (1 << 8)

/* Large array containing 2^18 characters */
char from[1 << 18];
char to[1 << 18];

int
main(void)
{
	int pipe_fds[2]; /* see `man pipe(3)`: `[0]` = read end, `[1]` = write end */
	size_t buf_sz  = sizeof(from);
	size_t written = 0;

	memset(from, 'z', buf_sz);

	if (pipe(pipe_fds) == -1) {
		perror("pipe creation");
		exit(EXIT_FAILURE);
	}

	/* the most complicated `memcpy` implementation ever! */
	while (buf_sz != 0) {
		ssize_t ret_w, ret_r;
		size_t write_amnt = MIN(buf_sz, WRITE_CHUNK);

		ret_w = write(pipe_fds[1], &from[written], write_amnt);
		if (ret_w < 0) {
			perror("write to pipe");
			exit(EXIT_FAILURE);
		}

		ret_r = read(pipe_fds[0], &to[written], ret_w);
		/* Question: why should this always be the case? */
		assert(ret_r == ret_w);

		/* Question: Describe at a high-level what `buf_sz` and `written` are doing here. */
		buf_sz  -= ret_w;
		written += ret_w;
	}

	/* `memcmp` is like `strcmp`, but for general (non-string) memory */
	assert(memcmp(from, to, sizeof(from)) == 0);

	printf("Successfully memcpyed through a pipe!\n");

	return 0;
}
```

Program output:
```
Successfully memcpyed through a pipe!
```

Pipes are really quite a bit more useful when they are used to coordinate between separate processes.
Thus, lets combine them with `fork`:

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/* Large array containing 2^20 characters */
char from[1 << 20];
char to[1 << 20];

int
main(void)
{
	int pipe_fds[2]; /* see `man 3 pipe`: `[0]` = read end, `[1]` = write end */
	pid_t pid;
	size_t buf_sz = sizeof(from);

	if (pipe(pipe_fds) == -1) {
		perror("pipe creation");
		exit(EXIT_FAILURE);
	}

	/* descriptors copied into each process during `fork`! */
	pid = fork();

	if (pid < 0) {
		perror("fork error");
		exit(EXIT_FAILURE);
	} else if (pid == 0) { /* child */
		ssize_t ret_w;

	    close(pipe_fds[0]); /* we aren't reading! */
		ret_w = write(pipe_fds[1], from, buf_sz);
		if (ret_w < 0) {
			perror("write to pipe");
			exit(EXIT_FAILURE);
		}
		assert((size_t)ret_w == buf_sz);
		printf("Child sent whole message!\n");
	} else { /* parent */
		ssize_t ret_r;

	    close(pipe_fds[1]); /* we aren't writing! */
		ret_r = read(pipe_fds[0], to, buf_sz);
		if (ret_r < 0) {
			perror("read from pipe");
			exit(EXIT_FAILURE);
		}
		/* Question: why should this always be the case? */
		assert((size_t)ret_r == buf_sz);

		printf("Parent got the message!\n");
	}

	return 0;
}
```

Program output:
```
inline_exec_tmp: inline_exec_tmp.c:50: main: Assertion `(size_t)ret_r == buf_sz' failed.
make[1]: *** [Makefile:22: inline_exec] Aborted (core dumped)
```

The *concurrency* of the system enables separate processes to be active at the same time, thus for the `write` and `read` to be transferring data through the pipe *at the same time*.
This simplifies our code as we don't need to worry about sending chunks of our data.

Note that we're `close`ing the end of the pipe that we aren't using in the corresponding processes.
Though the file descriptors are identical in each process following `fork`, each process does have a *separate* set of those descriptors.
Thus closing in one, doesn't impact the other.
Remember, processes provide *isolation*!

### The Shell

We can start to understand part of how to a shell might be implemented now!
Lets start with the more obvious: for each `|` in a command, the shell will create a new `pipe`.
It is a little less obvious to understand how the standard output for one process is hooked up through a `pipe` to the standard input of the next process.
To do this, the shell does the following procedure:

1. Create a `pipe`.
2. `fork` the processes (a `fork` for each process in a pipeline).
3. In the *upstream* process `close(STDOUT_FILENO)`, and `dup2` the writable file descriptor in the pipe into `STDOUT_FILENO`.
4. In the *downstream* process `close(STDIN_FILENO)`, and `dup2` the readable file descriptor in the pipe into `STDIN_FILENO`.

Due to this *careful* usage of `close` to get rid of the old standard in/out, and `dup` or `dup2` to methodically replace it with the pipe, we can see how the shell sets up the processes in a pipeline!

**Exercise**: write a small program that enables a process to use `printf`, and have the output of that go through a pipe to be `scanf`ed in the receiving process.
For example:

- Process 1: `printf("a = %d, b = %d", a, b);`
- Process 2: `scanf("a = %d, b = %d", &a, &b);`

Just remember that `printf` sends it output to `STDIN_FILENO`, and `scanf` reads its input from `STDIN_FILENO`, so you have to set those up properly!

## Signals

We're used to *sequential* execution in our processes.
Each instruction executes after the previous in the "instruction stream".
However, systems also require *exceptional* execution patterns.
What happens when you access memory that doesn't exist (e.g. `*(int *)NULL`); when you divide by `0`; when a user types `cntl-c`; when a process terminates; or when you make a call to access a descriptor which somehow is not accessible outside of the call^[We'll discuss the kernel later, and dual-mode execution to protect the implementation of these calls in OS.]?

UNIX's **signals** provide *asynchronous* execution in a process.
Regardless the previous instruction a process was executing, when a signal activates, a *signal handler* function will immediately activate.
Lets see what this looks like:

```c
#include <signal.h> /* sigaction and SIG* */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void
sig_fault_handler(int signal_number, siginfo_t *info, void *context)
{
	/* printf has problems here; see "The Dark Side of Signals" */
	printf("My downfall is the forbidden fruit at address %p.\n", info->si_addr);

	/* Question: what happens if we comment this out? */
	exit(EXIT_FAILURE);

	return;
}

int
main(void)
{
	sigset_t masked;
	struct sigaction siginfo;
	int ret;

	sigemptyset(&masked);
	sigaddset(&masked, SIGSEGV);
	siginfo = (struct sigaction) {
		.sa_sigaction = sig_fault_handler,
		.sa_mask      = masked,
		.sa_flags     = SA_RESTART | SA_SIGINFO /* we'll see this later */
	};

	if (sigaction(SIGSEGV, &siginfo, NULL) == -1) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}

	printf("Lets live dangerously\n");
	ret = *(int *)NULL;
	printf("I live!\n");

	return 0;
}
```

Program output:
```
Lets live dangerously
My downfall is the forbidden fruit at address (nil).
make[1]: *** [Makefile:22: inline_exec] Error 1
```

This is kind-of crazy.
We can actually execute in the signal handler when we access invalid memory!
We can write code to execute in response to a segmentation fault.
This is how Java prints out a backtrace -- something we will do in C soon.
The concepts in the above code:

- `sigset_t` this is a bitmap with a bit per signal.
    It is used to program the *signal mask*.
	While a specific signal is executing, which signals can occur during its execution?
	If a bit is set to `1`, then the signal will wait for the specific signal to *complete* execution, before its handler is called.
	However, if a signal's bit is set to `0`, then signals can *nest*: you can run a signal handler while you're in the middle of executing another signal handler.
	`sigemptyset` initializes all bits to `0`, while `sigaddset` sets the bit corresponding to a signal number (`SIGSEGV` in this case).
- `sig_fault_handler` is our signal handler.
    It is the function that is called asynchronously when the corresponding event happens.
	In the case of `SIGSEGV`, here, the signal handler is called whenever we access invalid memory (e.g. segmentation fault).
- `sigaction` is the function that enables us to set up a signal handler for a specific signal.

There are signals for quite a few events.
A list of all of the signals is listed in `man 7 signal` and `glibc` has decent [documentation](https://www.gnu.org/software/libc/manual/html_node/Signal-Handling.html).
Notable signals include:

- `SIGCHLD` - A notification that a child process has terminated, thus is awaiting `wait`
- `SIGINT` - The user typed `cntl-c`
- `SIGSTOP` & `SIGCONT` - Stop a child executing, and later continue its execution
- `SIGTSTP` - The user typed `cntl-z` (...or other causes)
- `SIGPIPE` - write to a pipe with no reader
- `SIGSEGV` - Invalid memory access (segmentation fault)
- `SIGTERM` & `SIGKILL` - Terminate a process; `SIGTERM` can be caught, but `SIGKILL` is [forceful, non-optional termination](https://turnoff.us/geek/dont-sigkill/)
- `SIGHUP` - The terminal attached to the shell that created us, itself terminated
- `SIGALRM` - A notification that time has passed
- `SIGUSR1` & `SIGUSR2` - User-defined signal handlers you can use for your own machinations

Each signal has a *default* behavior that triggers if you *do not* define a handler.
These are either to ignore the signal, terminate the process, to stop the process executing, and to continue its execution.

### Parent/Child Coordination with Signals

Another example for coordination between parent and child processes:

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> /* kill, pause */
#include <assert.h>
#include <sys/wait.h>

volatile int child_exited = 0;

void
sig_handler(int signal_number, siginfo_t *info, void *context)
{
	switch(signal_number) {
	case SIGCHLD: {
		printf("%d: Child process has exited.\n", getpid());
		fflush(stdout);
		child_exited = 1;
		break;
	}
	case SIGTERM: {
		printf("%d: We've been asked to terminate. Exit!\n", getpid());
		fflush(stdout);
		exit(EXIT_SUCCESS);
		break;
	}}

	return;
}

void
setup_signal(int signo)
{
	sigset_t masked;
	struct sigaction siginfo;
	int ret;

	sigemptyset(&masked);
	sigaddset(&masked, signo);
	siginfo = (struct sigaction) {
		.sa_sigaction = sig_handler,
		.sa_mask      = masked,
		.sa_flags     = SA_RESTART | SA_SIGINFO
	};

	if (sigaction(signo, &siginfo, NULL) == -1) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}
}

int
main(void)
{
	pid_t pid;

	setup_signal(SIGCHLD);
	setup_signal(SIGTERM);

	/*
	 * The signal infromation is inherited across a fork,
	 * and is set the same for the parent and the child.
	 */
	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		printf("%d - Heyo!\n", getpid());
		pause();
		printf("%d - post-pause\n", getpid());

		exit(EXIT_SUCCESS);
	}

	printf("%d: Parent asking child to terminate\n", getpid());
	kill(pid, SIGTERM); /* send the child the TERM signal */

	/* Wait for the value to be set! */
	while (!child_exited) ;

	assert(pid == wait(NULL));

	return 0;
}
```

Program output:
```
345961: Parent asking child to terminate
345961: Child process has exited.
```

*Note:* You want to run this a few times on your system to see the output.
The auto-execution scripts of the lectures might cause wonky effects here due to concurrency.

We now see a couple of new features:

- The `SIGCHLD` signal is activated in the parent when a child process exits.
- We can use the `kill` function to *send a signal* to a another process owned by the same user (e.g. `gparmer`).
- The `pause` call says to stop execution (to pause) until a signal is triggered.

**Question**: Please explain the control flow through this program.

A couple of additional important functions:

- `raise` will trigger a signal in the current process (it is effectively a `kill(getpid(), ...)`).
- `ualarm` will set a recurring `SIGALRM` signal.
    This can be quite useful if your program needs to keep track of time in some way.

### Tracking Time with Signals

Lets see `ualarm` in action:

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

volatile int timers = 0;

void
sig_handler(int signal_number, siginfo_t *info, void *context)
{
	timers++;

	return;
}

void
setup_signal(int signo)
{
	sigset_t masked;
	struct sigaction siginfo;
	int ret;

	sigemptyset(&masked);
	sigaddset(&masked, signo);
	siginfo = (struct sigaction) {
		.sa_sigaction = sig_handler,
		.sa_mask      = masked,
		.sa_flags     = SA_RESTART | SA_SIGINFO
	};

	if (sigaction(signo, &siginfo, NULL) == -1) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}
}

int
main(void)
{
	int t = timers;

	setup_signal(SIGALRM);
	ualarm(1000, 1000); /* note: 1000 microseconds is a millisecond, 1000 milliseconds is a second */
	while (t < 10) {
		if (timers > t) {
			printf("%d milliseconds passed!\n", t);
			t = timers;
		}
	}

	return 0;
}
```

Program output:
```
0 milliseconds passed!
1 milliseconds passed!
2 milliseconds passed!
3 milliseconds passed!
4 milliseconds passed!
5 milliseconds passed!
6 milliseconds passed!
7 milliseconds passed!
8 milliseconds passed!
9 milliseconds passed!
```

**Question**: Again, track and explain the control flow through this program.

`SIGKILL` and `SIGSTOP` are unique in that they *cannot be disabled*, and handlers for them cannot be defined.
They enable non-optional control of a child by the parent.

### The Dark Side of Signals

Signals are dangerous mechanisms in some situations.
It can be difficult to use them properly, and avoid bugs.
Signals complication data-structures as only functionality that is *re-eentrant* should be used in signal handlers, and they complicate the logic around all system calls as they can *interrupt* slow system calls.

**"Slow" system calls.**
Many library calls can *block*.
These are calls that cannot necessarily complete immediately as they might require concurrent events to complete.
For example, the `wait` call has to *block* and wait for the child to `exit`; and `read` has to block if there is no data available (yet) on a `pipe`.

But what happens if a signal is sent to a process *while* it is blocked?
By default, the system call will *wake up* and return immediately, even though the call's functionality has not been provided.
For example, `wait` will return even though a child didn't `exit`, and `read` returns despite not reading data.

So how do you tell the difference between the blocking function returning properly, or returning because it was interrupted by a signal?
The answer is, of course, in the `man` pages: the function will return an error (`-1`), and `errno` is set to `EINTR`.
Given this, we see the problem with this design: now the programmer must add logic for *every single system call* that can block to check this condition.
Yikes^[A hallmark of *bad design* is functionality that is not [*orthogonal* with existing functionality](https://www.youtube.com/watch?v=mKJcqvozfA8&t=2124s). When a feature must be considered in the logic for many other features, we're adding a significant complexity to the system.].

Luckily, UNIX provides a means to disable the interruption of blocking calls by setting the `SA_RESTART` flag to the `sa_flags` field of the `sigaction` struct passed to the `sigaction` call.
Note that the code above already sets this as I consider it a default requirement if you're setting up signal handlers.

**Re-entrant computations.**
Signal handlers execute by interrupting the currently executing instructions, regardless what computations they are performing.
Because we don't really know anything about what was executing when a signal handler started, we have to an issue.
What if an action in the signal handler in some way *conflicts* with the action it interrupted?
Conceptually, `printf` will copy data into a buffer before calling `write` to output it to standard output.
What if half-way through generating the data in `printf`, we execute a signal handler, and call `printf`...which happens to overwrite all of the hard work of the interrupted `printf`!
Any function that has these issues is called [*non-reentrant*](https://www.gnu.org/software/libc/manual/html_node/Nonreentrancy.html).
Yikes.
An example:

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

void
sig_handler(int signal_number, siginfo_t *info, void *context)
{
	/*
	 * Reset `errno`! In a real program, this might instead be a call that causes
	 * an error setting `errno` to whatever the error is for *that call*.
	 */
	errno = 0;

	return;
}

void
setup_signal(int signo)
{
	sigset_t masked;
	struct sigaction siginfo;
	int ret;

	sigemptyset(&masked);
	sigaddset(&masked, signo);
	siginfo = (struct sigaction) {
		.sa_sigaction = sig_handler,
		.sa_mask      = masked,
		.sa_flags     = SA_RESTART | SA_SIGINFO
	};

	if (sigaction(signo, &siginfo, NULL) == -1) {
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}
}

int
main(void)
{
	setup_signal(SIGUSR1);

	assert(read(400, "not going to work", 10) == -1);
	raise(SIGUSR1);
	printf("errno should be \"Bad file descriptor\", but has value \"%s\"\n", strerror(errno));

	return 0;
}
```

Program output:
```
errno should be "Bad file descriptor", but has value "Success"
```

The set of functions you *can* call in a signal handler (i.e. that are *re-entrant*) are listed in the manual page: `man 7 signal-safety`.
Notably these do *not* include the likes of

- `printf`/`snprintf`
- `malloc`
- `exit` (though `_exit` is fine)
- functions that set `errno`!
- ...

It is hard to do much in a program of any complexity without `snprintf` (called by `printf`), `malloc`, or use `errno`.
A very common modern way to handle this situation is to create a `pipe` into which the signal handlers `write` a notification (e.g. the signal number), while the main flow execution `read`s from the pipe.
This enables the main execution in our programs to handle these notifications.
However, this is only really possible and feasible when we get to `poll` later, and can block waiting for any of a number of file descriptors, including this pipe.

Note that in *most* cases, you won't see a bug due to using non-re-entrant functions in your signal handlers.
These bugs are heavily non-deterministic, and are dependent on exactly what instructions were interrupted when the signal activated.
This may feel like a good thing: buggy behavior is very rare!
But reality is opposite: rare, non-deterministic bugs become *very very hard to debug and test*.
Worse, these bugs that pass your testing are more likely to happen in customer's systems that might have different concurrency patterns.
So it is nearly impossible to debug/test for these bugs, and they are more likely to cause problems in your customer's environment.
Again, yikes.

# Libraries and System Calls

We're used to writing our own programs, and like to think of the code we write being relatively self-contained.
In contrast, quite a bit of *system programming* is about providing functionality that can be used by *any program on the system*.
What if you wanted to implement the world's best key-value store, and wanted to enable anyone to use your implementation!?

We think of these *shared* functionalities as falling into one of two categories:

1. *Libraries* that represent bodies of code that can be pull into our applications.
    You're already quite used to these!
	`libc` is the core library that provides all of the functions that we use when we include many of the headers in the class, for example, `#include <stdlib.h>`, `#include <stdio.h>`, or `#include <string.h>`.
2. *Services* which track different aspects of the system, and provide information to our programs.
    They tend to be programs that are always running on your system, and your programs can communicate with them to harness their functionality.

This lecture, we'll focus on libraries, but I want to give you a slightly more concrete understanding of services on a modern Linux system.
The `systemctl` command enables you to understand many of the services on the system (of which there are many: `systemctl --list-units | wc -l` yields `244` on my system).

```
$ systemctl --list-units
...
  cups.service           loaded active     running   CUPS Scheduler
...
  gdm.service            loaded active     running   GNOME Display Manager
...
  ssh.service            loaded active     running   OpenBSD Secure Shell server
...
  NetworkManager.service loaded active     running   Network Manager
...
  openvpn.service        loaded active     exited    OpenVPN service
...
```

I've pulled out a few selections that are easier to relate to:

- CUPS which is the service used to receive print jobs.
- [GDM](https://en.wikipedia.org/wiki/GNOME_Display_Manager) which manages your *graphical logins* on Ubuntu.
- OpenSSH which manages your *remote* logins through `ssh`.
- NetworkManager that you interact with when you select which wifi hotspot to use.
- OpenVPN which handles your VPN logins to the school network.
    You can see that the service is currently "exited" as I'm not running VPN now.

## Libraries - Goals and Overview

We'll cover *services* later in the class, and here we'll focus mainly on libraries.
The *goals* of libraries are to:

- Enable programs to leverage shared implementations of specific functionality (e.g. the contents of `string.h`).
- Provide a uniform set of programming APIs to programs.
- Attempt to save memory by making all programs in the system as small as possible by not taking memory for all of the library code in each program.
- Enable the system to upgrade both libraries and programs (for example, if a security compromise is found).

Libraries have two core components that mirror what we understand about C: *header files* that share the types of the library API functions and data, and the *code to implement* those APIs.
There are two main ways in which library code is integrated into programs:

- **Static libraries.**
    These are added into your program *when you compile it*.
- **Shared or dynamic libraries.**
    These are integrated into your program *when you run the program*.

We'll discuss each of these in subsequent sections

## Libraries - Header Files

We can see most of the header files in `/usr/include`, within which you'll find some familiar files:

```
$ ls /usr/include/std*
/usr/include/stdc-predef.h  /usr/include/stdint.h  /usr/include/stdio_ext.h  /usr/include/stdio.h  /usr/include/stdlib.h
```

Yes, this means that you have the source code for much of the standard library at your fingertips!
But how does `gcc` know to find all of these files when we use `#include <>`?
The following `gcc` incantation gives us a good idea:

```
$ gcc -xc -E -v -
...
#include <...> search starts here:
 /usr/lib/gcc/x86_64-linux-gnu/9/include
 /usr/local/include
 /usr/include/x86_64-linux-gnu
 /usr/include
End of search list.

```

These are all of the "include paths" that are searched, by default, when we do an `#include <>`.

> Aside: In `Makefiles`, we often want to add to tell the compiler to look for header files in our own files
> Thus, we update the include paths by adding multiple `-I<dir>` options to the `gcc` command line.
> We can see how this works:
>
> ```
> $ gcc -I. -xc -E -v -
> ...
> #include <...> search starts here:
> .
> /usr/lib/gcc/x86_64-linux-gnu/9/include
> ...
> ```
>
> The include paths are searched in order, so you can see that when we explicitly tell `gcc` to include a directory `-I.` here, we're asking to to check in the current directory (`.`) first.

## Linking Undefined Symbols to References Across Objects

The answer to how the code for libraries is provided is complicated, and has multiple options that represent trade-offs.
Before we can dive into that, we have to understand how object files (`*.o`) and binaries think about symbols and linking them together.

When your program uses a library's API, the library's code is *linked* into your program.
Linking is the act of taking any symbols (recall: functions and global variables) that are referenced, but not defined in your object (`*.o`) files, and the definition of those symbols in the objects that provide them.
We have to understanding the linking operation to dive into how libraries can be added into, thus accessible from your program.
We can see this process in action using the common set of programs for interpreting object files including `nm`, `objdump`, and `readelf`.

As an example, lets look at your `ptrie` implementation.
We know that each of the tests (in `tests/0?-*.c`) depends on your ptrie implementation in `ptrie.c`.
What does that look like?

```
$ nm tests/01-add.o
                 U _GLOBAL_OFFSET_TABLE_
000000000000022a T main
                 U printf
                 U ptrie_add
                 U ptrie_allocate
                 U ptrie_free
                 U __stack_chk_fail
0000000000000000 t sunit_execute
0000000000000000 d test
0000000000000184 T test_add
0000000000000144 T test_alloc
```

The left column is the address of the symbol, and the character in the middle column gives us some information about the symbol:

- `T` - this symbol is part of the code of the `*.o` file, and is visible outside of the object (i.e. it isn't `static`).
    Capital letters mean that the symbol is visible outside the object which simply means it can be linked with another object.
- `t` - this symbol is part of the code, but is *not visible* outside of the object (it is `static`).
- `d` - a global variable that is not visible.
- `D` - a global variable that is visible.
- `U` - an *undefined symbol* that must be provided by another object file.

For other symbol types, see `man nm`.

We can see what we'd expect: the test defines its own functions for (e.g. `main`, `test_add`), but requires the `ptrie_*` functions and `printf` to be provided by another object file.
Now lets check out the `ptrie` objects.

```
$nm ptrie.o
...
                 U calloc
                 U free
...
                 U printf
00000000000005ae T ptrie_add
00000000000003fc t ptrie_add_internal
0000000000000016 T ptrie_allocate
0000000000000606 T ptrie_autocomplete
00000000000000f8 T ptrie_free
...
                 U putchar
                 U puts
                 U strcmp
                 U strdup
                 U strlen
```

We can see that the `ptrie.o` object depends on other objects for all of the functions we're using from `stdio.h`, `malloc.h`, and `string.h`, provides all of the functions that are part of the public API (e.g. `ptrie_add`), and some other symbols (e.g. `ptrie_add_internal`) that *cannot be linked* to other object files.

After we link the `ptrie` into the test (with `gcc ... tests/01_add.o ptrie.o -o tests/01_add.test`),

```
$ nm tests/01_add.test | grep alloc
                 U calloc@@GLIBC_2.2.5
0000000000001566 T ptrie_allocate
00000000000013ad T test_alloc
```

Now we can see that there are no longer any undefined references (`U`) to `ptrie_allocate`, so the test object has now found that symbol within the `ptrie.o` object.
Additionally, we can see that some symbols (e.g. `calloc` here) are still undefined and have some mysterious `@@...` information associated with them.
You might guess that this somehow tells us that the function should be provided by the standard C library (`glibc` on Ubuntu).

*Where are we?*
OK, lets summarize so far.
Objects can have *undefined symbol references* that get *linked* to the symbols when combined with the objects in which they are defined.
How is this linking implemented?

First, we have to understand something that is a little *amazing*: all of our objects and binaries have a defined file format called the [Executable and Linkable Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) (ELF)^[ELF is a file format along side `html` for webpages, `.c` for C files, [`.png`](https://raw.githubusercontent.com/corkami/pics/master/binary/PNG.png) for images, and [`.pdf`](https://raw.githubusercontent.com/corkami/pics/master/binary/PDF.png) for documents. It just happens to contain all of the information necessary to link and execute a program! It may be surprising, but comparable formats even exist for [java (`.class`) files](https://github.com/corkami/pics/blob/master/binary/CLASS.png) as well.].

![A representation of the ELF binary program format by [Ange_Albertini](https://github.com/corkami/pics).](figures/elf.png)

We can confirm that these files are ELF with
```
$ xxd ptrie.o | head -n 1
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
```
`xxd` dumps out a file in hexadecimal format, and `head` just lets us filter out only the first line (i.e. at the head).
The first characters in most formats often denote the *type* of file, in this case an ELF file.

This reveals a cool truth: **programs are data**, plain and simple.
They happen to be data of a very specific format, ELF.
`nm`, `readelf`, and `objdump` are all programs that simply understand how to dive into the ELF format, parse, and display the information.

Now we get to an interesting part of the ELF objects:

```
$ readelf -r tests/01_add.o | grep ptrie
000000000151  001500000004 R_X86_64_PLT32    0000000000000000 ptrie_allocate - 4
000000000179  001600000004 R_X86_64_PLT32    0000000000000000 ptrie_free - 4
000000000191  001500000004 R_X86_64_PLT32    0000000000000000 ptrie_allocate - 4
0000000001dd  001800000004 R_X86_64_PLT32    0000000000000000 ptrie_add - 4
00000000021f  001600000004 R_X86_64_PLT32    0000000000000000 ptrie_free - 4
```

The `-r` flag here outputs the "relocation" entries of the elf object.

This output says that there are number of references to the `ptrie_*` functions in the code, and enumerates each of them.
You can imagine that these are simply all of the places in the code that these functions are called.
The first column gives us the offset into the ELF object where the reference to the function is made -- in this case where it is called.
For example, at the offset `0x151`^[Note: these are hexadecimal values, not base-10 digits.] into the ELF object, we have a call to `ptrie_allocate`.
This is important because the `tests/01_add.o` does not know where the `ptrie_allocate` function is yet.
It will only know that when it links with the `ptrie` implementation!

Lets check out what this looks like in the ELF object's code.
For this, we'll use `objdump`'s ability to dump out the "assembly code" for the object file.
It also will try and print out the C code that corresponds to the assembly, which is pretty cool.
```
$ objdump -S tests/01_add.o
...
sunit_ret_t
test_alloc(void)
{
 144:   f3 0f 1e fa             endbr64
 148:   55                      push   %rbp
 149:   48 89 e5                mov    %rsp,%rbp
 14c:   48 83 ec 10             sub    $0x10,%rsp
    struct ptrie *pt;

    pt = ptrie_allocate();
 150:   e8 00 00 00 00          callq  155 <test_alloc+0x11>
 155:   48 89 45 f0             mov    %rax,-0x10(%rbp)
...
```
I've printed out the addresses in the object around offset `0x151` because the first reference to `ptrie_allocate` is made there (see `readelf` output above).
We can see an instruction to `call` `test_alloc`.
However, you can see that the address that the binary has for that function is `00 00 00 00`, or `NULL`!
This is what it means for a function to be undefined (recall: the `nm` output for the symbol is `U`).

Now we can understand what a *linker*'s job is:

> **What does a Linker do?**
> A linker takes all of the undefined *references* to symbols from the *relocation records*, and rewrites the binary to populate those references with the address of that symbol (here updating `00 00 00 00` to the actual address of `test_alloc`).

In our `Makefile`s, we always use `gcc` both for compiling `*.c` files into objects, but also linking objects together into a binary.

## Static Libraries

A static library is essentially a collection of object files.
Static libraries have names of the form `lib*.a` and are created with the `ar` program from a collection of objects.
An example from the `ptrie` library (expanded from its `Makefile`) that creates `libptrie.a`:

```
$ ar -crs libptrie.a *.o
```

A static library file (`lib*.a`) is really just a file that contains a set of objects (much like a `.zip` or `.tar`/`.tgz` file of objects).
When we want to link a program to a static library, we need to use a few compiler flags:

```
gcc -o tests/01_add.test 01_add.o -L. -lptrie
```

The last two flags are the relevant ones:

- `-L.` says "look for static libraries in the current directory (i.e. `.`)", and
- `-lptrie`  says "please link me with the ptrie library (found in a file called `libptrie.a`).

Note that the linker already searches a few directories for libraries (i.e. default `-L` paths):

```
$ gcc -print-search-dirs | grep libraries | sed 's/libraries: =//' | tr -s ":" '\n'
/usr/lib/gcc/x86_64-linux-gnu/9/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../../x86_64-linux-gnu/lib/x86_64-linux-gnu/9/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../../x86_64-linux-gnu/lib/x86_64-linux-gnu/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../../x86_64-linux-gnu/lib/../lib/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../x86_64-linux-gnu/9/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../x86_64-linux-gnu/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../../lib/
/lib/x86_64-linux-gnu/9/
/lib/x86_64-linux-gnu/
/lib/../lib/
/usr/lib/x86_64-linux-gnu/9/
/usr/lib/x86_64-linux-gnu/
/usr/lib/../lib/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../../x86_64-linux-gnu/lib/
/usr/lib/gcc/x86_64-linux-gnu/9/../../../
/lib/
/usr/lib/
```

As many of these paths are in directories that any user can access, this is how the functionality of these libraries can be accessed by any program wishing to use them.
As we compile our `ptrie` library as a static library, you've already seen one example of these in use.

### Saving Memory with Shared Libraries

Static libraries do provide some facilities for trying to shrink the amount of memory required for library code.
If this was all done naively, then all of the object files in a static library could get loaded into a program with which it is linked.
This means that for $N$ programs that link with a library whose objects take $X$ bytes, we'll have $N \times X$ bytes devoted to storing programs on disk, and running the programs in memory (if they all run at the same time).
Some static libraries can be quite large.

Instead, static libraries are smarter.
If a static library contains multiple `.o` files, *only those object files that define symbols that are undefined in the program being linked with, are compiled into the program*.
This means that when designing a static library, you often want to break it into multiple `.o` files along the lines of different functionalities that separately used by different programs^[This is increasingly not true as many compilers support [Link-Time Optimization](https://en.wikipedia.org/wiki/Interprocedural_optimization#WPO_and_LTO) (LTO). This goes beyond the scope of this class.].

Some projects take this quite far.
For example [musl libc](https://www.musl-libc.org/) is a libc replacement, and it separates almost every single function into a separate object file (i.e. in a separate `.c` file!) so that only the exact functions that are called are linked into the program.

## Shared/Dynamic Libraries

Shared or dynamic libraries (for brevity, I'll call them only "dynamic libraries", but both terms are commonly used) are linked into a program *at runtime* when the program starts executing.

Recall that even executable binaries might still have undefined references to symbols.
For example, see `calloc` in the example below:
```
$ nm tests/01_add.test | grep calloc
                 U calloc@@GLIBC_2.2.5
```
Though we can execute the program `tests/01_add.test`, it has references to functions that don't exist in the program!
How can we possibly execute a program that has undefined functions; won't the calls to `calloc` here be `NULL` pointer dereferences?

To understand how dynamic linking works, lets look at the output of a program that tells us about dynamic library dependencies, `ldd`.
```
$ ldd tests/01_add.test
        linux-vdso.so.1 (0x00007ffff3734000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fb502adf000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fb502cfa000)
```
`ldd` also simply parses through the ELF program, and .
For now, we'll ignore the `linux-vdso`, but the other two entries are interesting.
We can see that the C standard library, `libc` is being provided by `/lib/x86_64-linux-gnu/libc.so.6`.
These are both dynamic libraries, as are most `*.so.?*` and `*.so` files.
If we check out that object we see that it provides `calloc`.
```
$ objdump -T /lib/x86_64-linux-gnu/libc.so.6  | grep calloc
000000000009ec90 g    DF .text  0000000000000395  GLIBC_2.2.5 __libc_calloc
000000000009ec90  w   DF .text  0000000000000395  GLIBC_2.2.5 calloc
```
So this library provides the symbols we require (i.e. the `calloc` function)!

But how does the `libc.so` library get linked into our program?
Diving in a little bit further, we see that `ld` is our program's "interpreter":
```
$ readelf --program-headers tests/01_add.test
Elf file type is DYN (Shared object file)
Entry point 0x1180
There are 13 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000000040 0x0000000000000040
                 0x00000000000002d8 0x00000000000002d8  R      0x8
  INTERP         0x0000000000000318 0x0000000000000318 0x0000000000000318
                 0x000000000000001c 0x000000000000001c  R      0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x00000000000008e8 0x00000000000008e8  R      0x1000
  LOAD           0x0000000000001000 0x0000000000001000 0x0000000000001000
                 0x0000000000000e55 0x0000000000000e55  R E    0x1000
...
```

A lot is going on here, but we see the reference to `ld-linux-x86-64.so.2` that we saw previously in the `ldd` output.
We can now see that the library (that I'll call simply `ld`) is a "program interpreter" (see "`Requesting program interpreter: /lib64/ld-linux-x86-64.so.2`") for our program^[We can even choose to use a different program interpreter, for example, `interp.so`, using `gcc -Wl,--dynamic-linker,interp.so`.].
This is the core of understanding how our program can execute while having undefined references to functions provided by another library.

When we call `exec`, we believe that the program we execute takes over the current process and starts executing.
This is not true!
Instead, if a program interpreter is defined for our program (as we see: it is), the interpreter program's memory is loaded along side our program, but then the interpreter is executed instead of our program!
It is given access to our program's ELF object, and `ld`'s job is to finish linking and loading our program.

But wait.
Why?
Why load `ld` to link our program instead of just running our program?
Because `ld` *also* loads and links all of the dynamic libraries (`.so`s) that our program depends on!!!

We can confirm that `ld` is loaded into our program by executing it in `gdb`, blocking it on breakpoint, and outputting its memory maps (`cat /proc/262648/maps` on my system):
```
555555554000-555555555000 r--p 00000000 08:02 527420                     /home/gparmer/repos/gwu-cs-sysprog/22/hw_solns/02/tests/01_add.test
555555555000-555555556000 r-xp 00001000 08:02 527420                     /home/gparmer/repos/gwu-cs-sysprog/22/hw_solns/02/tests/01_add.test
555555556000-555555557000 r--p 00002000 08:02 527420                     /home/gparmer/repos/gwu-cs-sysprog/22/hw_solns/02/tests/01_add.test
555555557000-555555558000 r--p 00002000 08:02 527420                     /home/gparmer/repos/gwu-cs-sysprog/22/hw_solns/02/tests/01_add.test
555555558000-555555559000 rw-p 00003000 08:02 527420                     /home/gparmer/repos/gwu-cs-sysprog/22/hw_solns/02/tests/01_add.test
7ffff7dbe000-7ffff7de3000 r--p 00000000 08:02 2235639                    /usr/lib/x86_64-linux-gnu/libc-2.31.so
7ffff7de3000-7ffff7f5b000 r-xp 00025000 08:02 2235639                    /usr/lib/x86_64-linux-gnu/libc-2.31.so
7ffff7f5b000-7ffff7fa5000 r--p 0019d000 08:02 2235639                    /usr/lib/x86_64-linux-gnu/libc-2.31.so
7ffff7fa5000-7ffff7fa6000 ---p 001e7000 08:02 2235639                    /usr/lib/x86_64-linux-gnu/libc-2.31.so
7ffff7fa6000-7ffff7fa9000 r--p 001e7000 08:02 2235639                    /usr/lib/x86_64-linux-gnu/libc-2.31.so
7ffff7fa9000-7ffff7fac000 rw-p 001ea000 08:02 2235639                    /usr/lib/x86_64-linux-gnu/libc-2.31.so
7ffff7fac000-7ffff7fb2000 rw-p 00000000 00:00 0
7ffff7fc9000-7ffff7fcd000 r--p 00000000 00:00 0                          [vvar]
7ffff7fcd000-7ffff7fcf000 r-xp 00000000 00:00 0                          [vdso]
7ffff7fcf000-7ffff7fd0000 r--p 00000000 08:02 2235423                    /usr/lib/x86_64-linux-gnu/ld-2.31.so
7ffff7fd0000-7ffff7ff3000 r-xp 00001000 08:02 2235423                    /usr/lib/x86_64-linux-gnu/ld-2.31.so
7ffff7ff3000-7ffff7ffb000 r--p 00024000 08:02 2235423                    /usr/lib/x86_64-linux-gnu/ld-2.31.so
7ffff7ffc000-7ffff7ffd000 r--p 0002c000 08:02 2235423                    /usr/lib/x86_64-linux-gnu/ld-2.31.so
7ffff7ffd000-7ffff7ffe000 rw-p 0002d000 08:02 2235423                    /usr/lib/x86_64-linux-gnu/ld-2.31.so
7ffff7ffe000-7ffff7fff000 rw-p 00000000 00:00 0
7ffffffde000-7ffffffff000 rw-p 00000000 00:00 0                          [stack]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
```
Recall that this dumps all of the memory segments of the process.

There are two important observations:

- `/usr/lib/x86_64-linux-gnu/libc-2.31.so` is present in the process so somehow got linked and loaded into the process.
- `/usr/lib/x86_64-linux-gnu/ld-2.31.so` is also present in the process; indeed it was the first code to execute in the process!

### Dynamic Loading Summary

Lets summarize the algorithm for executing a dynamically loaded program.

1. `exec` is called to execute a new program.
2. The program's ELF file is parsed, and if a "program interpreter" is set, the interpreter is instead executed.
3. In most cases, the interpreter is `ld`, and it is loaded into the process along with the target program.
4. Execution is started at `ld`, and it:

    1. Loads all of the dynamic libraries required by the program.
    2. Links the program such that all of its references to library symbols (i.e. `calloc` above) are set to point to library functions.
    3. Executes the initialization procedures (and eventually `main`) of our program.

This is all quite complex.
Why would we ever want all of this complexity over the simplicity of static libraries?

### Saving Memory with Dynamic Libraries

First, lets look at how much memory a library takes up.

```
$ ls -lh /usr/lib/x86_64-linux-gnu/libc-2.31.so
-rwxr-xr-x 1 root root 2.0M Dec 16  2020 /usr/lib/x86_64-linux-gnu/libc-2.31.so
```

So `libc` takes around 2MiB.
Other libraries take quite a bit more:

- Graphics card drivers (e.g. `libradeonsi_dri.so` is 23 MiB)
- Databases clients (e.g. `libmysqlclient.so` is 7.2 MiB)
- Languages (e.g. firefox's javascript engine, `libmozjs.so` is 11 MiB, and webkit's `libjavascriptcoregtk.so` is 26 MiB)
- ...

If $N$ programs must load in multiple libraries (totaling, say, $X$ MiB), then $N \times X$ MiB is consumed.
For context, my system is currently running 269 programs (`ps aux | wc -l`), so this memory can add up!

Dynamic libraries enable us to do a lot better.
The contents memory for the library is the same regardless the process it is present in, and an Operating System has a neat trick: it can make the same memory appear in multiple processes if it is identical *and* cannot be modified^[This sharing of library memory across processes is why dynamic libraries are also called *shared libraries.*].
As such, dynamic libraries typically only require memory for each library *once* in the system (only $X$), as opposed for each process.

**Static vs. dynamic library memory usage.**
When a program is linked with a static library, the result is relatively small as only the objects in the library that are necessary are linked into the program.
In contrast, a program that uses a dynamic library links in the entire library.
Thus, for a *single* program, a static library approach is likely going to be more memory-efficient.
However, in a system with *many* processes, the per-library memory, rather than per-process memory, will result in significantly less memory consumption.

# Appendix: C Programming Conventions

C gives you quite a bit of freedom in how you write your code.
Conventions are *necessary* in C because they bring order to the unrestricted freedom that allows code that is too complicated to debug, too clever to understand, and completely unmaintainable.
Conventions are a set of rules that we follow that inherently *limit* the way in which we write code.
The intention is to put [guard rails](https://en.wikipedia.org/wiki/Guard_rail) onto our development process and to only deviate from the conventions in exceptional cases.
An important aspect of this is that once you understand the conventions, it is *drastically easier to read the code* because you only have to consider how convention-based code is written.
This means that there is a significant importance to uniformly applying conventions.
Because of this, it is less important if one likes the conventions, than it is to adhere to them for uniformity.

Put another way, it is easy for *any* programmer to code themself into a corner where the program is more complex than our brains can handle.
Being an experienced programmer is *much less* about being able to handle more complexity, and much more about understanding that you need to spend significant development time *simplifying* your code.
Conventions are a set of codified simplifications, in some sense.
They provide your a set of rules (that you don't need to think about, you just do) to simplify the structure of your code.

OK, onward to conventions!

## Naming Conventions

There are many potential naming conventions, for example, [camel case](https://en.wikipedia.org/wiki/Camel_case), [snake case](https://en.wikipedia.org/wiki/Snake_case), and [hungarian notiation](https://en.wikipedia.org/wiki/Hungarian_notation).
Some are antiquated (hungarian), but most are a matter of taste and convention.

> **Convention.**
> Most C uses lower-case `snake_case` for variables, types, and functions, capital `SNAKE_CASE` for constants and macros.

Stick to this convention.
A new convention will likely not feel "right" because it is new and different.
Remember that they are subjective, but the aim to apply them uniformly.

For names that are visible across `.c` files (i.e. functions in a `.h` file), use names that are the shortest they can be to convey the functionality.
Within functions, names can be short, and here there are a few conventions as well:

- `ret` is a variable that stores a return value (i.e. a value that will later be used as in `return ret;`).
- `i`, `j`, `k`, `l` are iterator variables.

## Namespacing

When you create a global variable, type, or function, you're creating a symbol that can be accessed for any `.c` file.
This has a significant downside: if you choose a name that is already used in a library, or that is common, you might get a linker error along the lines of:

```
multiple definition of `foo'; /tmp/ccrvA8in.o:a.c:(.text+0x0): first defined here
...
collect2: error: ld returned 1 exit status
```

This means that the naming choices each application and library makes can easily cause such a problem.

> **Convention.**
> Thus, the convention is that you should "namespace" your functions and variables by pre-pending them with a functionality-specific prefix.
> For example, a key-value store might have all of its types, variables, and functions prefixed with `kv_`, for example, `kv_get`.

## Headers and Visibility

C does not have classes and the rules therein to support per-class/object encapsulation.
Instead, C views each `.c` file as a unit of visibility.
Functions and global variables (known as "symbols") can be hooked up with references to them in other objects by the linker.
However, it is possible to write code in a `.c` file that is *not visible* outside of that file.
In contrast to the linking error above emphasizing good naming, if a symbol is marked as `static`, no other `.c` file will be able to reference that symbol.

``` c
static int global_var;
static void foo(void) { }
```

No other `.c` file can define code that will be able to access `global_var` nor call `foo`.
You can see the list of all symbols within a `.c` file that are accessible outside of it by compiling into a `.o` file (using `gcc -c ...`), and `nm -g --defined-only x.o | awk '{print $3}'` (replace `x` with your object file name).

> **Convention.**
> All functions and variables that are not part of the *public interface* with which to interact with the `.c` file should be `static` so that they cannot be used from other objects.

This convention enables `.c` files to support encapsulation.
It is also important to support encapsulation at compile time (which is before linking).
We want our compiler to tell us if we try and access a symbol we shouldn't be able to access.
Header files are `#include`d into `.c` files, thus providing access to the symbol names and types, so that the `.c` file can compile while referencing symbols in another.

> **Convention.**
> The focus of header files should be to provide the types for the data and functions of a public interface, and nothing more.

When we include a file, it just copies the file at the point of the `#include`.
Thus, if the same header is included separately into two `.c` files, then it effectively replicates the header's contents into the two `.o` files.
This can cause problems if the two `.o` files are linked together.
An example:

`a.c`:
``` c
#include "c.h"
```

`b.c`:
``` c
#include "c.h"

int main(void) { return 0; }
```

`c.h`:
``` c
int foo(void) { return -1; }
```

After compiling these files, we get
```
$ gcc -c a.c -o a.o
$ gcc -c b.c -o b.o
$ gcc a.o b.o -o test
/usr/bin/ld: b.o: in function `foo':
b.c:(.text+0x0): multiple definition of `foo'; a.o:a.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

Because `foo` was replicated into each `.o` file, when they were linked together the linker said "I see two `foo`s!".
Note that we would have avoided this if we put only the function declaration (i.e. the type information) in the `.h` file (e.g. `int foo(void);`).

> **Convention.**
> Never put global data in a `.h` file.
> Only put type information (e.g. function declarations) in `.h` files^[There are sometimes optimization-driven motivations to place function definitions in `.h` files. In such cases, the functions *must* be marked `static` to avoid the above problem.].

## What to Include?

You *must* include the header files that provide the functionality that you rely on and use.
This sounds obvious, but it is *very* easy to mess this up.
A couple of examples:

*Example 1: Implementing a library.*
You're implementing a library, and you have a `.h` file for it that exposes the types of the library's functions.
You test your library, and it works wonderfully -- victory!
However, because it is a library, it might be used by other people's code, and all they are going to do is `#include <your_lib.h>`, and expect everything to work.

What if the following happened?
Your `your_lib.h` file does this:

``` c
/* ... */
size_t my_function(struct my_type *t);
/* ... */
```

In your test file, you do this:

``` c
#include <stddef.h>
#include <your_lib.h>

/* ... */
```

Your tests of your library work wonderfully!
But if the user of your library simply does:

``` c
#include <your_lib.h>
/* ... */
```

The user will get a compiler error that says it doesn't understand what `size_t` is!
The core problem is that your library's header file didn't include all of the headers it relies on!
Instead, it should updated to look like so:

``` c
#include <stddef.h> /* we require `size_t` */
/* ... */
size_t my_function(struct my_type *t);
/* ... */
```

A good way to defensively code to try to minimize the chance that this error happens, you should include your header files *before* the rest of the header files.
The only real solution is to be very careful, and always include what headers are necessary for the functionality you depend on.

*Example 2: implementing a program or test.*
A `.c` file, lets say `my_prog.c`, includes some header file, `head.h`.
It compiles just fine.
But if `my_prog.c` relies on some header file -- lets say `dep.h` -- that is only included itself in the `head.h` file, then if `head.h` is updated and removes the include, it will break any program that depends on it!
In this case, `my_prog.c` should explicitly include `dep.h`: include the headers that are required for your functionality!

*How do you know which headers to include?*
If you're using a system function, the `man` page for it is explicit about which headers provide the necessary functionality.
If you're implementing a library in the class, we are explicit about what headers are required.

## Depth = Complexity

Syntactic nesting, or depth often significantly increase the cognitive complexity in C^[...and in most languages that don't support first class functions/closures.].
Two examples of this:

**Depth through indentation nesting.**
Each additional nesting level (each nested set of `{}`) adds another condition that you must keep in your mind^[Short term memory only holds 7 $\pm$ 2 items, an each nesting level expands at least one.] when understanding the code.
This includes the conditions, or looping conditions associated with the nesting level.
This is *a lot* to keep in your head, and makes developing and reading code quite a bit more difficult.

What can you do to avoid this nesting?
*First*, return early for errors.
Instead of the following code...
``` c
if (a) {
	/* normal code */
} else {
	/* error code */
	return;
}
```
...do the following...
``` c
if (!a) {
	/* error code */
	return;
}
/* normal code */
```
Though seemingly trivial, it enables you to separate your understanding of error conditions from the complexity of the following code.
Try to perform as much error checking as soon as it possibly can be done so that you can switch into "normal case" thinking.

*Second*, if you nest deeper than four levels of brackets, then pull code out into appropriately-named functions.
Long functions aren't necessarily bad, but *complex* functions are.

> **Convention.**
>
> 1. Separate error handling where possible, and put it as close to the top of the function as possible.
> 2. If you nest brackets deeper than 4 levels, pull logic out into separate, appropriately named functions.

I use indentation levels in C of eight visual spaces to make it visually clear when I'm indenting too much.

**Depth through field nesting.**
It can be quite difficult to follow code that nests access to fields.
For example:
``` c
a.b->c[d].e = f.g->h;
```
Of course, this is fake code, but even if it wasn't, keeping track of the types of each of the levels of increasing depth of field reference is nearly impossible.
This will make writing the code fickle and challenging, and reading the code, needlessly complex.

The main means we have to remove this complexity is to

1. Create an set of typed variables

    ``` c
	struct *b   = &a.b;
	struct *c[] = b->c;
	struct *g   = &f.g;

	c[d].e = g->h;
	```

    It is up to you to decide where it is right to provide an "intermediate" type for a certain depth in the fields, and when to directly dive into a few fields.
2. If any the structs (or sub-structs, or arrays) represents an API layer, then you can call a function that performs the necessary operation on that part of the data-structure.
    This both provide abstraction over the data access, and documents the types.

> **Convention.**
> When faced with walking through nested composite data-structures, use intermediate variables to make types explicit, or functions to abstract operations.

## Object Orientation: Initialization, Destruction, and Methods

C is an imperative programming language as it came before Object Orientation (OO) took hold.
That said, the OO concept of [encapsulation](https://en.wikipedia.org/wiki/Encapsulation_(computer_programming)) behind an [abstract data type](https://en.wikipedia.org/wiki/Abstract_data_type) is a universal requirement of programming in complex environments.
Though C doesn't support this explicitly, conventions can provide much of the same benefit.
Such an abstraction requires

1. a *datatype* that stores the information required for the abstraction (think: a list, a key-value store, etc...),
2. a set of *methods* to operate on that datatype, and
3. means to *create and destroy* objects of that datatype.

If we wanted to provide a key-value datatype, `kv`, we'd provide these in C by

1. providing a `struct kv` for the datatype in which the rest of the data is contained,
2. publicly providing (in `kv.h`) a set of functions with names all prefixed with `kv_`, that take the `kv` to operate on as the first argument (e.g. `void *kv_get(struct kv *kv, int key);`) -- this first argument acts as "`this`" in Java, and
3. allocate and deallocate functions, for example, `struct kv *kv_alloc(...)` and `void kv_free(struct kv *kv)`^[We also often provide an initialization function that initializes a `struct kv` instead of allocating a new one (e.g. `int kv_init(struct kv *kv)`). These are useful since C allows stack and global allocation of the datatype, and we want to provide a means to initialize one of those allocations.].

C can support polymorphism, inheritance, dependency injection and composition as well, but we'll leave that for a future discussion.

> **Convention.**
>
> 1. Meticulously name your header files, your structure, and the methods to act on the datatype to associate them with each other.
> 2. Pass the object to be acted on as the first argument to each of the "methods" to act as the C equivalent of "`this`".
> 3. Provide allocation and deallocation functions for the datatype.

With this, we encourage the client (user of the API) to think of the datatype as encapsulated.
