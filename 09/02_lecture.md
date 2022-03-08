
# Programming Dynamic Libraries and System Calls

We saw how libraries enable the sharing of functionality between programs.
Static libraries are convenient, and only required objects are linked with your program at compile time.
In contrast, dynamic libraries are linked at runtime *before* the `main` of your program executes.

## Plugins and Dynamic Library APIs

In addition to enabling shared library code, dynamic libraries also enable explicit means to load libraries programmatically.
Namely, UNIX provdes an API that enables your process to *load* a dynamic library into itself.
This is kinda crazy: a running process with a set of code and data can add a library with more code and data into itself dynamically.
This effectively means that your program can change its own code as it executes by adding and removing dynamic libraries.
Being able to load library code into your process as your program executes (at runtime) is an instance of *self-modifying code*, and it enables a few very cool features including:

- *dynamic update of services* in which new versions of key computations in a service can be dynamically loaded in (and old versions eventually removed),
- *plugin architectures* enable different interesting functionalities to be loaded into a process to provide unique functionalities -- some of the most popular image and video processing programs (e.g. `ffmpeg`) enable different formats to be handled with separate dynamic libraries, thus enabling the processing of any image or video by simply loading a new library, and
- *languages*, such as `python`, dynamically load libraries (many or most are written in C) into a process when the python code `import`s them.
	Since the `import`s are part of the code, the corresponding dynamic libraries must be loaded by the python runtime dynamically.
    This enables your dynamic (slow) python code to load and leverage fast C libraries.

![Example programmatic uses of dynamic libraries.](figures/09_dl_uses.svg)

This API is provided by the linker (`ld.so`) that loaded the elf object for the currently executing process.
What does the API that enables this dynamic loading look like?

- `handle = dlopen(file_path, flags)` - Open and load in a dynamic library.
    This returns a handle to the library that you should pass into the latter functions.
    You can read about the `flags` in `man dlopen`.
	Unless otherwise noted, I'd recommend using `RTLD_NOW | RTLD_LOCAL`^[What are "flags"? The notion of "flags" is pretty confusing. Flags are often simply a set of bits (in this case, the bits of the `int`) where each bit specifies some option. In this case, there's a bit for `RTLD_NOW` which says to load all of the symbol dependencies now, and a separate bit for `RTLD_LOCAL` which says that subsequent libraries should not see this library's symbols. In short, these are just single bits in the "flags" integer, thus they can be bitwise or-ed together (thus the `|`) to specify options -- they are simply a way to specify a set of options.].
	Returns `NULL` if it cannot find the dynamic library.
- `dlclose(handle)` - Close and unload a library that was previously loaded.
- `symb_ptr = dlsym(handle, symbol)` - pass in a `symbol` which is a string holding a function or variable name in the dynamic library.
    It should return a pointer to that symbol, which you can then call (if it is a function pointer), or dereference if it is a variable.

```c
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int
main(void)
{
	void *handle;
	typedef int *(*bar_fn_t)(int a);
	bar_fn_t bar;
	int *ret;

	/* Open a dynamic library: the example is in ../08/libexample/ */
	handle = dlopen("08/libexample/libexample.so", RTLD_NOW | RTLD_LOCAL);
	if (!handle) return EXIT_FAILURE;

	/*
	 * Lets find the function "bar" in that dynamic library.
	 * We know that `bar` is the function: int *bar(int a)
	 */
	bar = dlsym(handle, "bar");
	if (!bar) return EXIT_FAILURE;

	/* bar allocates an int, populates it with the argument + 2 and returns it */
	ret = bar(40);
	printf("%d\n", *ret);
	free(ret);

	return 0;
}
```

As `dlsym` returns a pointer to the symbol passed in (as a string), it means that you end up seeing a *lot* of function pointers when you're using plugin infrastructures.
See (a) in the image below.

We can also use `dlsym` to ask for a symbol in *any of the installed libraries*.
To do so, we pass in the "special pseudo-handles" `RTLD_DEFAULT` or `RTLD_NEXT`.

```c
#define _GNU_SOURCE /* added this due to `man 3 dlsym` */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	typedef int (*write_fn_t)(int fd, char *buf, size_t s);
	write_fn_t w;
	char *buf = "Look at me! Diving into your dynamic libraries...pulling out the symbols\n";

	/* Now lets find `write`! */
	w = dlsym(RTLD_DEFAULT, "write");
	if (!w) return EXIT_FAILURE;
	w(STDOUT_FILENO, buf, strlen(buf));

	return 0;
}
```

Though this example doesn't have much of a point, as we can directly call `write`, thus have to go through the symbol we retrieved from `dlsym`,
See (b) in the image below.

![Three programmatic means to interact with dynamic libraries.](figures/09_dl_api.svg)

## Interposing Libraries on Library APIs

One powerful mechanism that dynamic libraries enable is to *interpose* on library function calls.
Your program might believe that it calls `malloc` when it invokes the function "malloc", but you can orchestrate it so that it instead calls a `malloc` in your library!
You can then, if you choose, invoke the normal library calls, thus why this is known as *interposing* on the library calls.
But how?

The environment variable `LD_PRELOAD` can be used to specify a dynamic library to load into the process to *interpose* on any library calls.
A trivial use of this might be to *log* (i.e. write out to a file) all of a set of library calls.
We'll use it to add intelligence on the memory allocation paths.

When we use `LD_PRELOAD=lib`, the symbols in the dynamic library that we specify in `lib` have priority over those of other libraries.
Thus, if we include a `malloc` function definition, when the application calls `malloc`, it will call *our* malloc!
In the image above, (c) demonstrates using `LD_PRELOAD`.

In [`malloclog`](https://github.com/gwu-cs-sysprog/lectures/tree/main/09/malloclog/), we can see a library that interposes on `malloc` and `free`, and prints information to standard error.
This includes the following:

``` c
typedef void *(*malloc_fn_t)(size_t s);
static malloc_fn_t malloc_fn;

/* This `malloc` will override libc's `malloc` */
void *
malloc(size_t sz)
{
	void *mem;

	/* if we don't yet know where libc's malloc is, look it up! */
	if (malloc_fn == NULL) {
		/*
		 * Lets find the `malloc` function in libc! The `RTLD_NEXT`s
		 * specify that we don't want to find our *own* `malloc`, we'd like to
		 * search starting with the *next* library.
		 */
		malloc_fn = dlsym(RTLD_NEXT, "malloc");
		assert(malloc_fn);
	}
	/* call libc's `malloc` here! */
	mem = malloc_fn(sz);
	fprintf(stderr, "malloc(%ld) -> %p\n", sz, mem);

	return mem;
}
```

We do the same for `free`.

**Question:** Why can't we just call `malloc` directly instead of doing this complex thing with `dlsym`?

We can use it on a simple program that simply calls `malloc` and `free` on the allocated memory.
To test this out, in [`malloclog`](https://github.com/gwu-cs-sysprog/lectures/tree/main/09/malloclog/):

```
$ make
$ LD_PRELOAD=./libmalloclog.so ./main.bin
malloc(4) -> 0x55fe6cd492a0
free(0x55fe6cd492a0)
```

This shows us that dynamic libraries are *programmable* and that we can use them in interesting ways to coordinate between libraries, provide new functionalities, and to interpose on functionalities.
For example, many of the largest companies use `LD_PRELOAD` to swap out the `malloc` implementation for one that is higher performance.

Note that running a program that uses `LD_PRELOAD` environment variables are more difficult to use in `gdb`.
The [`ldpreload_w_gdb.sh`](https://github.com/gwu-cs-sysprog/lectures/tree/main/09/malloclog/ldpreload_w_gdb.sh) script demonstrates how to use both technologies together.

## System Calls

We've talked about the many resources that are provided by the *system*.
These include files, pipes, and domain sockets.
But we've been vague about what the system is.
Though we'll leave the details for a later course, lets dive in to what constitutes the "system" a little bit.
