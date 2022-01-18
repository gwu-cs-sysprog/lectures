
# Intermediate C

## Objectives

- Recalling C types with a focus on pointers.
- Focus on more advanced features like `void *`s and function pointers.
- Practice thinking about C programs as memory.

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

You can choose the values explicitly (e.g. `enum grades { MON = 1, TUES = 2, WED = 3, THURS = 4, FRI = 5};`).

**Modifiers**

- `unsigned` - variables that cannot be negative.
    Given that variables have a fixed bit-width, they can use the extra bit ("negative" no longer needs to be tracked) to instead represent numbers twice the size of `signed` variants.
- `signed` - signed variables.
    You don't see this modifier as much because `char`, `int`, `long` all default to `signed`.
- `long` - Used to modify another type to make it larger in some cases.
	`long int` can represent larger numbers and is synonymous with `long`.
	`long long int` (or `long long`) is an even larger value!
- `static` - this variable should not be accessible *outside of the .c file* in which it is defined.
- `const` - an immutable value.
	We won't focus much on this modifier.
- `volatile` - this variable should be "read from memory" every time it is accessed.
    Confusing now, relevant later, but not a focus.

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

	/* now I shouldn't access `.burger` in `f_eggs` */
	f_eggs.num_eggs = 10;
	/* This is just syntax for structure initialization. */
	f_burger.burger = (struct hamburger) {
		.num_burgers = 5,
		.cheese      = 1,
		.num_patties = 1
	};
	/* now shouldn't access `.num_eggs` in `f_burger` */

	printf("Size of union:  %ld\nSize of struct: %ld\n",
		   sizeof(union food), sizeof(struct all_food));

	return 0;
}
```

We can see the effect of the `union`: The size is `max(fields)` rather than `sum(fields)`.
What other examples can you think of where you might want `union`s?

> An aside on syntax:
> The structure initialization syntax in this example is simply a shorthand.
> The `struct hamburger` initialization above is equivalent to:
>
> ``` c
> f_burger.burger.num_burgers = 5;
> f_burger.burger.cheese      = 1;
> f_burger.burger.num_patties = 1;
> ```
>
> Though since there are so many `.`s, this is a little confusing.
> We'd typically want to simply as:
>
> ``` c
> struct hamburger *h = &f_burger.burger;
> h->num_burgers = 5;
> h->cheese      = 1;
> h->num_patties = 1;
> ```
>
> More on `->` in the next section.

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

static int value;

/* here, the `*` is part of the type "int *", i.e. a pointer to an `int` */
void
foo(int *ptr)
{
	/*
	 * Here, the `*` is the dereference operation, and
	 * gives us the value that is pointed to.
	 */
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

We can see that *pointer arithmetic* (i.e. doing addition/subtraction on pointers) does the same thing as array indexing plus a dereference.
That is, `*(a + 1) == a[1]`.
For the most part, arrays and pointers can be viewed as very similar, with only a few exceptions^[Arrays differ from pointers as 1. the `sizeof` operator on an array variable gives the total size of the array (including all elements) whereas for pointers, it returns the size of the pointer (i.e. 8 bytes on a 64 bit architecture); 2. `&` on an array returns the address of the first item in the array whereas for pointers, it returns the address of the pointer; and 3. assignments to pointers end up initializing the pointer to an address, or doing math on the pointer whereas for arrays, initialization will initialize items in the array, and math/assignment after initialization are not allowed.].

*Pointer arithmetic should generally be avoided in favor of using the array syntax.*
One complication for pointer arithmetic is that it does not fit our intuition for addition:

```c
#include <stdio.h>

int
main(void)
{
	int a[] = {6, 7, 8, 9};
	char b[] = {'a', 'b', 'c', 'd'};
	/*
	 * Calculation: How big is the array?
	 * How big is each item? The division is the number of items.
	 */
	int num_items = sizeof(a) / sizeof(a[0]);
	int i;

	for (i = 0; i < num_items; i++) {
		printf("idx %d @ %p & %p\n", i, a + i, b + i);
	}

	return 0;
}
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

#### Example

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

#### Generic Pointer Types

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

Data-structures often aim to store data of any type (think: a linked list of anything).
Thus, in C, you often see `void *`s to reference the data they store.

#### Relationship between Pointers, Arrays, and Arrows

Indexing into arrays (`a[b]`) and arrows (`a->b`) are redundant syntactic features, but they are very convenient.

- `&a[b]` is equivalent to `a + b` where `a` is a pointer and `b` is an index.
- `a[b]` is equivalent to `*(a + b)` where `a` is a pointer and `b` is an index.
- `a->b` is equivalent to `(*a).b` where `a` is a pointer to a variable with a structure type that has `b` as a field.

Generally, you should always try and stick to the array and arrow syntax were possible, as it makes your intention much more clear when coding than the pointer arithmetic and dereferences.

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
```c
#include <malloc.h>

int
main(void)
{
	int *a = malloc(sizeof(int));
	/* Error: did not check return value! */
	*a = 1;
	free(a);

	return 0;
}
```

- *Dangling pointer.*
    If you maintain a pointer to a chunk of memory that you `free`, and then dereference that pointer, bad things can happen.
	The memory might have already been re-allocated due to another call to `malloc`, and is used for something completely different in your program.
	It is up to you as a programmer to avoid `free`ing memory until all references to it are dropped.
```c
#include <malloc.h>

int
main(void)
{
	int *a = malloc(sizeof(int));
	if (a == NULL) return -1;
	free(a);

	/* Error: accessing what `a` points to after `free`! */
	return *a;
}
```

- *Memory leaks.*
    If you remove all references to memory, but *don't* `free` it, then the memory will *never* get freed.
	This is a memory leak.
```c
#include <malloc.h>

int
main(void)
{
	int *a = malloc(sizeof(int));
	if (!a) return -1;
	a = NULL;
	/* Error: never `free`d `a` and no references to it remain! */

	return 0;
}
```

- *Double `free`.*
    If you `free` the memory twice, bad things can happen.
	You could confuse the memory allocation logic, or you could accidentally `free` an allocation made after the first `free` was called.
```c
#include <malloc.h>

int
main(void)
{
	int *a = malloc(sizeof(int));
	if (!a) return -1;
	free(a);
	free(a);
	/* Error: yeah, don't do that! */

	return 0;
}
```

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
