
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
