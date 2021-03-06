
# System Calls and Memory Management

We've talked about the many resources that are provided by the *system*.
These include files, pipes, and domain sockets.
But we've been vague about what the system is.
Though we'll leave the details for a later course, lets dive in to what constitutes the "system" a little bit.

## System Calls

We've claimed many times that *the system* provides pipes, domain sockets, files, the file system, etc...
What is "the system"?
First, lets start from what we know: is this system just a dynamic library?
To some extent this makes sense.

**Question:**

- The dynamic libraries we've seen provide many of the functions we've used, so why *couldn't* they provide *all* of the functionality we require!?
    For example, why can't `read`, `open`, etc... all be simply provided solely by dynamic libraries?

A few observations:

1. Dynamic libraries are loaded into a process, but *what provides the process itself*?
2. Some resources, like `pipe`s, *involve multiple processes*, thus there must be something in the system beyond specific processes.
3. *Hardware is limited* -- a system only has a limited amount of DRAM (memory) -- and something has to determine how to hand it out to processes.

![The OS kernel's relationship to processes and libraries. Each OS has a kernel, in this case the Linux Kernel. The kernel provides resources such as file systems, channels (domain sockets, `pipe`s, etc...), and it implements processes (including `fork`, `exec`, `wait`, etc...). The kernel is separated from processes so that a failure in a process won't spread beyond the process. This means that when a process wants to interact with the kernel, it must make a special *system call*. This behaves like a function call, but it maintains isolation of the kernel from the process.  This is a system with four processes. The second is making a `write` system call, the third is using `printf` which is implemented in `libc` as part of the stream API in `stdio.h`. The stream implementation makes a `write` system call to send the data to the kernel (e.g. to display on our terminal). On the far right, we can see a process that doesn't even have any libraries and makes system calls directly.](figures/10_kernel_syscalls.svg)

The *system*, as we've discussed it so far, is called the *kernel*.
One of the main components of every operating system is the kernel.
Every process in the system uses the kernel's functionality.
It provides the logic for *most* of the calls we've learned in the class, `fork`, `exec`, `pipe`, `read`, `write`, `open`, `creat`, etc... and also most of the resources we've discussed including channels, domain sockets, pipes, files, and the filesystem.
It is some of the most trusted code on the system.
If it has a bug, the whole system can crash -- or worse!

We have to be able to call functions in the kernel from any process.
This means we want to make what seem like function calls to the kernel.
And we want to make sure that the faults of one process don't spread to another!
This means that somehow the kernel must be isolated from each process.

**System calls** are the mechanism that behaves like a function call (and looks like one to us!), but that switches over to the kernel.
System calls are a special instruction in the instruction set architecture (Intel x86 or ARM) that triggers the switch to the kernel.
When the kernel's done with its functionality, it can return (via a special instruction) to the process.

So system calls *behave* like function calls, but inherently involves *switches* from the process and to the kernel and back.

**Question:**

- So why does UNIX decide what functionality to put in a library, and what to put in the kernel?

### System Calls and Processes

In UNIX, system calls are available in `unistd.h`, so whenever you include that file to use a function, you know the function is likely a direct system call!
You can also tell if a UNIX function is a system call by looking at the *sections* of `man` (from `man man`):

```
MANUAL SECTIONS
    The standard sections of the manual include:

    1      User Commands
    2      System Calls
    3      C Library Functions
    4      Devices and Special Files
    5      File Formats and Conventions
    6      Games et. al.
    7      Miscellanea
    8      System Administration tools and Daemons
```

**Questions:**

- Do `man write`.
    What is this documentation for?
- Do `man 2 write`.
    What is this documentation for?
	What does the `2` mean?

    You can always tell which section you're looking at in the header line of the `man` output: `WRITE(2)` shows that we're reading about `write` in the "System Calls" section.

- Is `malloc` a system call or a C Library Function?

We know what a system call looks like, as we've used them many times throughout the class.
Lets look at the various ways to make the same system call.
First, the normal way:

```c
#include <unistd.h>

int
main(void)
{
	write(1, "hello world\n", 12);

	return 0;
}
```

The C standard library even provides you a "generic" way to invoke *any* system call.
The `syscall` function lets you make an integer-identified system call.
Each system call on the system is identified with a unique integer.
In this case, we'll use `SYS_write`.

```c
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
	syscall(SYS_write, 1, "hello world\n", 12);

	return 0;
}
```

Lets look at a few of these integer values to see what's happening here:

```c
#include <stdio.h>
#include <sys/syscall.h>

int
main(void)
{
	printf("write: %d\nread:  %d\nopen:  %d\nclose: %d\n",
		SYS_write, SYS_read, SYS_open, SYS_close);

	return 0;
}
```

Note, these integer values are *not* related to descriptors (e.g. `STDIN_FILENO == 0`).
The system calls in Linux (and in all operating systems!) are each assigned an integer with which to identify them.
You can see all of the [system calls and their integer numbers](https://filippo.io/linux-syscall-table/), and some documentation in [`man syscalls` (link)](https://manpages.debian.org/unstable/manpages-dev/syscalls.2.en.html).

**Exercise:**

- Find three interesting system calls that you didn't know about.

OK, now lets do a little more of a deep-dive.
You won't be familiar with the Intel's/AMD's x86-64 assembly language that follows, but the comments on the right give you the jist.

```asm
.data				 ; Add some global variables - our "hello world" message
msg:
    .ascii "hello world\n"
    len = . - msg	 ; magic length computation "here" - start of `msg`

.text				 ; The code starts here!
    .global _start

_start:              ; Note: the %rxx values are registers, $x are constants
    movq  $1,   %rax ; Syscall's integer value, 1 == write, now set up the arguments...
    movq  $1,   %rdi ; Arg 1: file descriptor 1
    movq  $msg, %rsi ; Arg 2: the string!
    movq  $len, %rdx ; Arg 3: the length of the string
    syscall			 ; Kick off the system call instruction!

    movq  $60,  %rax ; System call #60 == exit
    xorq  %rdi, %rdi ; Arg 1: xor(x, x) == 0, pass NULL as argument!
    syscall
```

We compile this assembly with `make` (in `10/`).
Program output:

```
hello world
```

We can see that the requirements of making a system call are quite minimal!
A special instruction (on x86-64, `syscall`) provides the magical incantation to make what feels like a function call *to the kernel*.

How does this work?
Recall that ELF, the file format for objects and programs includes many pieces of information necessary to execute the program.
Among them, the *starting or entry address* which is the initial instruction to start executing when the program runs.

```
$ readelf -a ./asm_syscall  | grep "Entry"
  Entry point address:               0x401000
$ readelf -a ./asm_syscall  | grep "401000"
  Entry point address:               0x401000
...
     6: 0000000000401000     0 NOTYPE  GLOBAL DEFAULT    1 _start
```

We can see that the "entry point address" for the binary is `0x401000`, and we can see that the `_start` symbol has that address.
The `exec` logic of the kernel will set the program to start executing at that `_start` address.

Note that this same analysis rings true for the normal C system call code above (not only for the assembly)!
The `_start` function is where all execution starts in each program.
This explains why our program above labels its code with `_start`!

## Observing System Calls

The `strace` program lets us monitor the system calls a program makes.
Running int on the assembly program above:

```
$ strace ./asm_syscall
execve("./asm_syscall", ["./asm_syscall"], 0x7fff3c06ae10 /* 50 vars */) = 0
write(1, "hello world\n", 12hello world
)           = 12
exit(0)                                 = ?
+++ exited with 0 +++
```

This likely has what you'd expect: We see the `exec` that kicks off the program, the `write`, and, finally, the `exit`.
You'd likely expect the same from the normal C program that also writes out `hello world`.

```
$ trace ./normal_syscall
execve("./normal_syscall", ["./normal_syscall"], 0x7ffc036c1520 /* 50 vars */) = 0
brk(NULL)                               = 0x5638ed59d000
arch_prctl(0x3001 /* ARCH_??? */, 0x7ffd8bfb49c0) = -1 EINVAL (Invalid argument)
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=138102, ...}) = 0
mmap(NULL, 138102, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7efddf831000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\360A\2\0\0\0\0\0"..., 832) = 832
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\237\333t\347\262\27\320l\223\27*\202C\370T\177"..., 68, 880) = 68
fstat(3, {st_mode=S_IFREG|0755, st_size=2029560, ...}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7efddf82f000
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\237\333t\347\262\27\320l\223\27*\202C\370T\177"..., 68, 880) = 68
mmap(NULL, 2037344, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7efddf63d000
mmap(0x7efddf65f000, 1540096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7efddf65f000
mmap(0x7efddf7d7000, 319488, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19a000) = 0x7efddf7d7000
mmap(0x7efddf825000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e7000) = 0x7efddf825000
mmap(0x7efddf82b000, 13920, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7efddf82b000
close(3)                                = 0
arch_prctl(ARCH_SET_FS, 0x7efddf830540) = 0
mprotect(0x7efddf825000, 16384, PROT_READ) = 0
mprotect(0x5638eb775000, 4096, PROT_READ) = 0
mprotect(0x7efddf880000, 4096, PROT_READ) = 0
munmap(0x7efddf831000, 138102)          = 0
write(1, "hello world\n", 12hello world
)           = 12
exit_group(0)                           = ?
+++ exited with 0 +++
```

Whaaaaa!?
If you look carefully, at the bottom we see the expected output.
But we also see a *lot* of system calls previous to that.
Between when we start execution at `_start`, and when we execute in `main`, quite a lot happens.

**Question:**

- What do you think is causing all of these system calls?
    What is the program doing before running `main`?

![Why are there so many system calls in a simple C program? Lets compare what happens in the C versus assembly programs. The process on the left is `normal_syscall` above, while `asm_syscall` is on the right. The assembly system call avoids much of the C runtime support, thus starts execution at `_start`, and makes system calls directly. In contrast, the C program starts execution in the dynamic linker `ld` (i.e. at `_start` in `ld`) -- step (1). `ld` links in the `libc` dynamic library (2), then executes `_start` in our program (3). Our program actually calls `libc` (4) before executing `main` to do library initialization, which then calls `main`. After `main` returns (5), it returns into `libc`, which then terminates our program with the `_exit` system call. So why are there so many system calls made in the C program? There's a lot of execution that happens outside of our logic! Most of the system calls are from `ld`.](figures/10_bootup_syscalls.svg)

## System Call Overhead

There are many reasons why some functionality might be in a library, and accessed via function calls, or in the kernel, and accessed with system calls.

Lets look at an example of memory allocation.
We can, of course, use `malloc` and `free` to allocate and free memory.
But we can also call `mmap` and `munmap` that make *system calls* to allocate and free memory.

**Questions:**

- Which will be *faster*, `malloc`/`free` or `mmap`/`munmap`?
- How much overhead would you *guess* a system call has?

```c
#include "10/timer.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#define ITER 256

int
main(void)
{
	int i;
	unsigned long long start, end;
	int *mem;

	/* How many cycles does `write` take? */
	start = cycles();
	for (i = 0; i < ITER; i++) {
		mem = malloc(256);
		mem[0] = 42;
		free(mem);
	}
	end = cycles();
	printf("\nmalloc + free overhead (cycles): %lld\n", (end - start) / ITER);

	/* How many cycles does `fwrite` take? */
	start = cycles();
	for (i = 0; i < ITER; i++) {
		mem = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		mem[0] = 42;
		munmap(mem, 256);
	}
	end = cycles();
	printf("\nmmap + munmap overhead (cycles): %lld\n", (end - start) / ITER);

	return 0;
}
```

> What is a "cycle"?
> When you read that you have a 3GHz, that essentially says that the internal "clock" that drives the logic in the processor cycles by at 3 billion cycles per second.
> That's a *lot*.
> Each cycle is 1/3 a nanosecond.
> A cycle is the smallest unit of measurement on a processor, and the `cycles` function we implement here gives us how many cycles have elapsed since the system boots.

We can also look at the output APIs like `write` and compare them to the stream APIs that buffer their output.

**Questions:**

- If we write a byte to a file using `write` or to a stream (via a `FILE`) using `fwrite`, what performance would you expect?
- How about if we did an `fflush` after the `fwrite`?

```c
#include "10/timer.h"

#include <unistd.h>
#include <stdio.h>

#define ITER 256

int
main(void)
{
	int i;
	unsigned long long start, end;

	/* How many cycles does `write` take? */
	start = cycles();
	for (i = 0; i < ITER; i++) write(1, " ", 1);
	end = cycles();
	printf("\nwrite overhead (cycles): %lld\n", (end - start) / ITER);

	/* How many cycles does `fwrite` take? */
	start = cycles();
	for (i = 0; i < ITER; i++) fwrite(" ", 1, 1, stdout);
	end = cycles();
	printf("\nfwrite (stream) overhead (cycles): %lld\n", (end - start) / ITER);

	/* How many cycles does `fwrite + fflush` take? */
	start = cycles();
	for (i = 0; i < ITER; i++) {
		fwrite(" ", 1, 1, stdout);
		fflush(stdout);
	}
	end = cycles();
	printf("\nfwrite + fflush overhead (cycles): %lld\n", (end - start) / ITER);

	return 0;
}
```

## Library vs. Kernel Trade-offs in Memory Allocation

We've seen that

1. the kernel is there to facilitate access to hardware resources (memory, processing, etc...),
2. the kernel provides processes (`fork`, `exec`) and inter-process interactions (`wait`, `pipe`, etc...) as these functionalities require implementation beyond any single process,
3. functions in dynamic libraries are much faster to invoke as system calls have significant overhead.

This leads us to the conclusion that for performance, we'd like to implement as many functions as possible in libraries, and to rely on the kernel when processes require modifications (`exec`, `fork`), when there are multiple processes involved (IPC), or when the finite resources of the system must be split up between processes (`open`, `creat`, `sbrk`).

Here, we're going to dive into the design of the *memory management* functionalities in UNIX to better understand the trade-offs.
Specifically, *how does `malloc` and `free` work*!?

### Library Tracking of Memory

So `malloc`, `calloc`, `realloc`, and `free` are implemented in a library (in `libc`), and when we request memory from them, they might call `sbrk` (or `mmap` on some systems) to get memory from the kernel, but from that point on, they will manage that memory themselves.
What does this mean?
When the *heap* doesn't have enough free memory to satisfy a `malloc` request, we call the kernel to *expand the heap*.
To do so, we ask the kernel for memory via `sbrk` or `mmap`, and we generally get a large chunk of memory.
For example, we might ask `sbrk` for 64KB of memory with `mem = sbrk(1 << 16)`^[Note, this is just $2^16$, using shifts to emulate the power operator.].
Then it is `malloc`'s job to split up that memory into the smaller chunks that we'll return in each subsequent call to `malloc`.
When memory is `free`d, it is returned to the pool of memory which we'll consider for future allocations.
Because we're *reusing* freed memory, we're avoiding system calls when the heap contains enough `free`d memory to satisfy a request.

![In (a), we see the heap with red rectangles that designate allocated memory. (b) we get a `malloc` request. First we check to see if there is any span of unallocated memory we can use to satisfy the request. (c) shows what we must avoid: the overlapping of two different allocations. As we cannot find a sufficient span of memory to satisfy the `malloc` request, in (d), we expand the heap using `sbrk`. (e) shows how we can satisfy the request from the newly expanded heap memory. ](figures/10_mem_splitting.svg)

This is what it means that our `malloc`/`free` code in the `libc` library need to track the memory in the heap.
We need to track *where allocated memory is located* and *where freed spans of memory are located*, to enable intelligent allocation decisions for future `malloc` calls.

**Questions:**

- What data-structures should we use to track allocated/freed memory?
- Do you foresee any challenges in using data-structures to track memory?

### Malloc Data-structures

The `malloc` and `free` implementation must track which chunks of memory are allocated, and which are freed, but it cannot use `malloc` to allocate data-structures.
That would result in `malloc` calling itself recursively, and infinitely!
If `malloc` requires memory allocation for its tracking data-structures, who tracks the memory for the tracking data-structures?
Yikes.

We solve this problem by allocating the tracking data-structures along with the actual memory allocation itself.
That is to say, we allocate a *header* that is a struct defined by `malloc` that is *directly before* an actual memory allocation.
Because the header is part of the same contiguous memory chunk, we allocate the header at the same time as we allocate memory!
When memory is freed, we use the header to track the chunk of memory (the header plus the free memory directly following the header) in a linked list of free chunks of memory called a *freelist*.

![Memory allocations through various sets of details. The red boxes are allocated memory, white box is memory that we've retrieved in the heap from `sbrk`, the green boxes are the *headers* data-structures that track free chunks of memory we can use for future allocations, and they are arranged in a linked list called *freelist*, and orange boxes are headers for allocated chunks of memory thta track how large that allocation is so that when we `free` it, we know large the chunk of freed memory will be.](figures/10_malloc.svg)

- (a) the heap with some memory allocated, and some free spans of memory.
- (b) we need to make a data-structure to track where the free spans of memory are, so we add a structure into each of the free chunks, and use it to construct a linked list of the free chunks.
- (c) When we `malloc`, we can find the free chunk large enough for the allocation, remove it from the freelist, ...
- (d) ...and return it!
- (e) If we want to free that memory, we must understand somehow (i.e. must have a structure that tracks) the size of the allocation, so that we can add it back onto the freelist.
- (f) We can track the size of each allocation in a *header* directly before the memory we return.
    We are tracking the memory in the same contiguous span of memory we use for the allocation!
- (g) So when we free, we're just adding that memory back onto the freelist.
- (h) If we ever receive a `malloc` request that can't be satisfied by the freelist, *only then* do we make a system call to allocate more heap.

### Exercise: `smalloc`

Look at the [`10/smalloc.c` (link)](https://github.com/gwu-cs-sysprog/lectures/blob/main/10/smalloc.c) file for a simple implementation of a `malloc`/`free` that assumes *only allocations of a specific size*.
This means that we don't need to track the sizes of allocations and free chunks in the headers, simplifying our implementation.
Build, and run the program:

```
$ make
$ ./smalloc.bin
```

It currently implements this simple `malloc`!
Read through the `smalloc` and `sfree` functions, and the `main`.
Answer `QUESTION 1-3`.
Then, move on to `QUESTION 4`.
In `expand_heap`, we make the system call to allocate more memory from the system with `sbrk`.
We need to set up that newly allocated heap memory to include memory chunks that we can allocate with `smalloc`.
Answering how this is done is the core of `QUESTION 4`.
