
# Libraries

Up till now, we've been focusing on UNIX programming APIs, and how to interact with various aspects of the system.
Part of this discussion has been how various programs can be orchestrated to cooperate.
Pipelines enable larger functionalities to be composed out of multiple programs in which the output of one goes to the input of the next.
System servers provide services to clients that use IPC to harness their functionality.
This enables servers that control system resources (like wifi) to let many clients have limited and controlled access to those resources.

The next few chapters dive under the hood of processes.
We'll investigate

1. how programs are represented and how to understand their format,
2. how programs are organized and can share code, and
3. how they interact with the system.

How do we think about our system's programs?
We're used to writing our own programs, and like to think of the code we write being relatively self-contained.
In contrast, quite a bit of *system programming* is about providing functionality that can be used by *any program on the system*.
What if you wanted to implement the world's best key-value store, and wanted to enable anyone to use your implementation!?

We think of these *shared* functionalities as falling into one of two categories:

1. *Libraries* that represent bodies of code that can be pull into our applications.
    You're already quite used to these!
	`libc` is the core library that provides all of the functions that we use when we include many of the headers in the class, for example, `#include <stdlib.h>`, `#include <stdio.h>`, or `#include <string.h>`.
2. *Services* which track different aspects of the system, and provide information to our programs.
    They tend to be programs that are always running on your system, and your programs can communicate with them to harness their functionality.
	These include services that receive print requests, that service `ssh` sessions, that provide your `vpn` connection, and that display your graphical user interface, and many other functions.
	We've already seen many of the IPC and communication foundations that make these tick.

This chapter, we'll focus on libraries as a foundation for sharing software in systems.

## Libraries - Goals and Overview

Before we start, consider:

- When you `#include` a ton of files, do you think that all of the related functionality is compiled into your program?
    That would mean that *every* program has the functionality compiled into it.
- If so, how much memory and disk space do you think that will take?
- Do you think there are other options?
    What might they be?

The *goals* of libraries are to:

- Enable programs to leverage shared implementations of specific functionality (e.g. the contents of `string.h`).
- Provide a uniform set of programming APIs to programs.
- Attempt to save memory by making all programs in the system as small as possible by not taking memory for all of the library code in each program.
- Enable the system to upgrade both libraries and programs (for example, if a security compromise is found).

Libraries are collections of functionality that can be used by many different programs.
They are code that expand the functionality of programs.
This differs from services which are separate programs.
Libraries have two core components that mirror what we understand about C:

1. *header files* that share the types of the library API functions and data, and
2. the *code to implement* those APIs.

There are two main ways library code is integrated into programs:

- **Static libraries.**
    These are added into your program *when you compile it*.
- **Shared or dynamic libraries.**
    These are integrated into your program, dynamically, *when you run the program*.

We'll discuss each of these in subsequent sections

## Libraries - Header Files

We can see most of the header files in `/usr/include`, within which you'll find some familiar files:

```
$ ls /usr/include/std*
/usr/include/stdc-predef.h  /usr/include/stdint.h  /usr/include/stdio_ext.h  /usr/include/stdio.h  /usr/include/stdlib.h
```

Yes, this means that you have the source code for much of the standard library at your fingertips!
But how does `gcc` know to find all of these files when we use `#include <...>`?
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
- `D` - a global variable that is visible.
- `d` - a global variable that is not visible.
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

First, we have to understand something that is a little *amazing*: all of our objects and binaries have a defined file format called the [Executable and Linkable Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) (ELF)^[ELF is a file format for binary programs in the same way that  `html` is the data format for webpages, `.c` for C files, [`.png`](https://raw.githubusercontent.com/corkami/pics/master/binary/PNG.png) for images, and [`.pdf`](https://raw.githubusercontent.com/corkami/pics/master/binary/PDF.png) for documents. It just happens to contain all of the information necessary to link and execute a program! It may be surprising, but comparable formats even exist for [java (`.class`) files](https://github.com/corkami/pics/blob/master/binary/CLASS.png) as well.].

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

### Linking Summary

Lets back up and collect our thoughts.

- Our programs are collections of a number of ELF object files.
- We've seen that some symbols in an object file are *undefined* if they are functions or variables that are not defined in a `.c` file.
- Though their types are defined in header files, their implementation must be found somewhere.
- Each reference to an undefined symbol has a *relation entry* that enables the linker to update the reference once it knows the ELF object that provides it.
- When another ELF exports the symbol that is undefined elsewhere, they can be *linked* to turn the function calls of undefined functions into function calls, and references to global variables be actual pointers to memory.

This is the *foundation for creating large programs out of small pieces*, and to enable some of those pieces to be *shared between programs*.

## Static Libraries

A static library is simply a collection of ELF object (`*.o`) files created with `gcc -c`.
They are collected together into a static library file that has the name `lib*.a`.
You can think of these files as a `.zip` or `.tar` file containing the ELF objects.

Static libraries of the form `lib*.a` and are created with the `ar` (archive) program from a collection of objects.
An example from the `ptrie` library (expanded from its `Makefile`) that creates `libptrie.a`:

```
$ ar -crs libptrie.a *.o
```

So a static library is just a collection of ELF objects created by our compilation process.
If we are writing a program that wants to *use* a library, first you make sure to include its header file(s).
Then, when compiling, we have to tell the compiler which library we want to link with by using a few compiler flags:

```
$ gcc -o tests/01_add.test 01_add.o -L. -lptrie
```

The last two flags are the relevant ones:

- `-L.` says "look for static libraries in the current directory (i.e. `.`)" -- other directories can be included, and a few default directories are used, and
- `-lptrie`  says "please link me with the `ptrie` library" which should be found in one of the given directories and is found in a file called `libptrie.a`.

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

### Saving Memory with Static Libraries

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

Shared or dynamic libraries (for brevity, I'll call them only "dynamic libraries", but both terms are commonly used) are linked into a program *at runtime* when the program starts executing as part of `exec`.

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
Thus, for a *single* program, a static library approach is almost always going to be more memory-efficient.

However, in a system with *many* executing programs, the per-library memory, rather than per-process memory, will result in significantly less memory consumption.
Requiring only a *single copy* of a *larger* dynamic library shared between all processes, uses less memory than *multiple copies* of *smaller* compilations of the library in each program.

### Explicitly Managing Dynamic Libraries

Libraries are great for providing shared functionality to multiple programs.
Dynamic libraries can also be used *explicitly* to enable additional capabilities.

1. The environment variable `LD_PRELOAD` can be used to specify a dynamic library to load into the process to *interpose* on any library calls.
    This enables you to write functions for common `libc` or other library APIs and have your functions be called by the program *instead* of the other library calls.
	You can then, if you choose, invoke the normal library calls, thus why this is known as *interposing* on the library calls.
	A trivial use of this might be to *log* (i.e. write out to a file) all of a set of library calls.
	I've used this to track all calls to the `malloc` API to implement a janky `valgrind` -- you could write your own `valgrind` variant that watches only the memory allocation API by tracking all `malloc` and `free` calls in a hash table to make sure that each chunk of `malloc`ed memory is freed exactly once.
2. An API enables your process to *load* a dynamic library into itself.
    This is kinda crazy: a running process with a set of code and data can add a library with more code and data into itself dynamically.

Being able to load library code into your process as your program executes (at runtime) is an instance of *self-modifying code*, and it enables a few very cool features including:

- *dynamic update of services* in which new versions of a service can be dynamically loaded in (and old versions eventually removed),
- *plugin architectures* enable different interesting functionalities to be loaded into a process to provide unique functionalities -- some of the most popular image and video processing programs (e.g. `ffmpeg`) enable different formats to be handled with separate libraries, thus enabling the processing of any image or video by simply loading a new library, and
- *languages*, such as `python`, dynamically load libraries (many or most are written in C) into a process when the python code `import`s them.
    This enables your dynamic (slow) python code to load and leverage fast C libraries.

This API is provided by the linker (`ld.so`) that loaded the elf object for the currently executing process.
What does the API that enables this dynamic loaded look like?

- `handle = dlopen(filename, flags)` - Open and load in a dynamic library.
    This returns a handle to the library that you should pass into the latter functions.
    You can read about the `flags` in `man dlopen`, but I'd recommend using `RTLD_NOW | RTLD_LOCAL`^[What are "flags"? The notion of "flags" is pretty confusing. Flags are often simply a set of bits (in this case, the bits of the `int`) where each bit specifies some option. In this case, there's a bit for `RTLD_NOW` which says to load all of the symbol dependencies now, and a separate bit for `RTLD_LOCAL` which says that subsequent libraries should not see this library's symbols. In short, these are just single bits in the "flags" integer, thus they can be bitwise or-ed together (thus the `|`) to specify options -- they are simply a way to specify a set of options.].
- `dlclose(handle)` - Close and unload a library that was previously loaded.
- `symb_ptr = dlsym(handle, symbol)` - pass in a `symbol` which is a string holding a function or variable name in the dynamic library.
    It should return a pointer to that symbol, which you can then call (if it is a function pointer), or dereference if it is a variable.
