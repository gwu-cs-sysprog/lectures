
# Processes

Programming is hard.
Really hard.
When you write a program, you have to consider complex logic and the design of esoteric data-structures, all while desperately trying to avoid errors.
It is an exercise that challenges all of our logical, creative, and risk management facilities.
As such, programming is the act of *methodically conquering complexity* with our *ability to abstract*.
We walk on the knives-edge of our own abstractions where we can barely, just barely, make progress and engineer amazing systems.

Imagine if when programming and debugging, we had to consider the actions and faults of *all other* programs running in the system?
If your program crashed because one of your colleagues' program had a bug, how could you make progress?

Luckily, we stand on the abstractions of those that came before us.
A key abstraction provided by systems is that of *isolation* - that one program cannot arbitrarily interfere with the execution of another.
This isolation is a core provision of Operating Systems (OSes), and a core abstraction they provide is the **process**.
At the highest-level, each process can only corrupt its own memory, and a process that goes into an infinite loop cannot prevent another process for executing.

A process has a number of properties including:

- It contains the set of memory that is only accessible to that process.
    This memory includes the code, global data, stack, and dynamically allocated memory in the *heap*.
    Different processes have *disjoint* sets of memory, thus no matter how one process alters its own memory, it won't interfere with the data-structures of another.
- A number of *descriptors* identified by integers that enable the process to access the "resources" of the surrounding system including the file system, networking, and communication with other processes.
    When we `printf`, we interact with the descriptor to output to the terminal.
    The OS prevents processes from accessing and changing resources they shouldn't have access to.
- A set of unique identifiers including a process identifier (`pid`).
- The "current working directory" for the process, analogous to `pwd`.
- A owning user (e.g. `gparmer`) for whom the process executes on the behalf of.
- Each process has a *parent* - the process that created it, and that has the ability and responsibility oversee it (like a normal, human parent).

Throughout the class we'll uncover more and more of these properties, but in this lecture, we'll focus on the lifecycle of a process, and its relationship to its parent.
Processes are the abstraction that provide isolation between users, and between computations, thus it is the core of the *security* of the system.

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
- `getpid`, `getppid` - get the unique identifier (the Process ID, or pid) for the process, or for its parent.
- `exit` - This is the function to terminate a process.
- `wait` - A parent awaits a child exiting.
- `exec` - Execute a new *program* using the current process^[This is a little bit like when an agent transforms from a normal citizen in the Matrix.].

Lets be a little more specific.
A *program* is an executable file.
We see them in the file system (for example, see `ls /bin`), and create them when we compile our programs.
When you compile your C programs, the generated file that you can execute, is a program.
A *process* is an *executing* program with the previously mentioned characteristics (disjoint memory, descriptors to resources, etc...).

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

### Non-determinism Everywhere: Concurrency

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

### `fork` Questions

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

### An Aside: Creating Processes with `posix_spawn`

`fork` and `exec` are not the only functions to execute a program.
`posix_spawn` also enables the creation of a new process that execute a given program.
`posix_spawn` performs three high-level actions:

1. `fork` to create a new process,
2. a set of "file actions" that update and modify the files/descriptors for the new process, that are specified in an array argument to `posix_spawn`, and
3. `exec` to execute a program and pass arguments/environmental variables.

It is strictly *more limited* in what it can do than `fork` and `exec`, but this is often a feature not a bug.
`fork` is really hard to use well, and can be quite confusing to use.
It is considered by some to be a [flawed API](https://www.microsoft.com/en-us/research/uploads/prod/2019/04/fork-hotos19.pdf).
Thus the focus of `posix_spawn` on specific executing a new program can be quite useful to simply programs.

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
