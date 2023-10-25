
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

![`exec` replaces the previous program executing in this process with the `exec`ed program. Not shown here: maintaining the descriptors, current working directory, etc... of the program previously executing in this process.](figures/exec_agentxform.gif)

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

I think that we likely have a decent intuition about what the command-line arguments are`:

```
$ ls /bin /sbin
```

The `ls` program takes two arguments, `/bin` and `/sbin`.
How does `ls` access those arguments?

Lets look at a *chain of programs* that `exec` each other.
The first program (that you see here) is called `inline_exec_tmp`, and the programs `03/args?.c` are subsequently executed.

![A chain of processes `exec`ing each other, and passing arguments. We only print them out in the 3rd program, `args2.bin`.](figures/exec_chain.png)

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

`args1.c` is

``` c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

char *prog = "./03/args2.bin";

int
main(int argc, char *argv[])
{
	int i;
	char **args; 		/* an array of strings */

	printf("Inside %s\n", argv[0]);

	/* lets just pass the arguments on through to args2! */
	args = calloc(argc + 1, sizeof(char *));
	assert(args);

	args[0] = prog;
	for (i = 1; i < argc; i++) {
		args[i] = argv[i];
	}
	args[i] = NULL; /* the arguments need to be `NULL`-terminated */

	if (execvp(prog, args)) {
		perror("exec");

		return EXIT_FAILURE;
	}

	return 0;
}
```

...and `args2.c` is

``` c
#include <stdio.h>

int
main(int argc, char *argv[])
{
	int i;

	printf("Inside %s\n", argv[0]);

	for (i = 0; i < argc; i++) {
		printf("arg %d: %s\n", i, argv[i]);
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

A common use of environment variables is the "home" directory in your shell.
How is this implemented?

```
$ cd ~
```

The `~` means "my home directory".
To understand what directory is a user's home directory, you can `getenv(HOME)`!

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

	printf("pid %d\nglobal (RO):\t%p\nglobal:      \t%p\nstack:      \t%p\nheap:      \t%p\nfunction:\t%p\n",
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
