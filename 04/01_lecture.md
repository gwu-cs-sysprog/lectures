
# Programming Process Resources

We've discussed the APIs for process manipulation and program execution, but now we're going to start discussing how processes can manipulate the resources it has access to.
Two of the key UNIX mechanisms that processes must use effectively are

- the current working directory (reported by `pwd`),
- the ability manipulate and use their *descriptors* -- often called "file descriptors" though they are more general than only interfacing with files, and
- controlling and leveraging the exceptional control flow provided by *signals*.

## Current Working Directory

Each process has a "working directory" which is used as the base for any *relative paths* in the file system.
All file system paths are either *absolute paths* -- in which case they begin with `/` and specify the path from the "root", or *relative paths*.
Relative paths are quite frequently used when we interact with the shell.
Every time you type `cd blah/`, you're saying "please change the current working directory to `blah/` which is a directory in the current working directory.
Similarly, `cat penny_the_best_pup.md` looks up the file `penny_the_best_pup.md` in the current working directory.

This is backed by a simple API:

- `getcwd` which lets you get the current process' working directory, and
- `chdir` which enable the process to change its working directory.

```c
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	char *wd = getcwd(NULL, 0);

	assert(wd);
	printf("Current directory: %s\n", wd);
	free(wd);

	if (chdir("..") == -1) {
		perror("chdir");
		abort();
	}
	wd = getcwd(NULL, 0);
	printf("New current dir: %s\n", wd);
	free(wd);

	return 0;
}
```

The `cd` command in shells is actually *not a program*, and is instead a shell-internal function.
You can confirm this using the `which` program, which tells you the location of a program:

```
$ which ls
/bin/ls
$ which pwd
/bin/pwd
$ which cd
$
```

`cd` isn't a program, so `which` can't tell us where it is!

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
		 * Didn't write everythign we wanted, better call write again sending
		 * data starting at `&hi[amnt]`, of length `hi_sz - amnt`.
		 */
	} else if (amnt == hi_sz) {
		/*
		 * Wrote out everything! Wooo!
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
		ssize_t rest = buf_sz, offset = 0;

	    close(pipe_fds[1]); /* we aren't writing! */
		while (rest > 0) {
			ret_r = read(pipe_fds[0], &to[offset], rest);
			if (ret_r < 0) {
				perror("read from pipe");
				exit(EXIT_FAILURE);
			}
			rest   -= ret_r;
			offset += ret_r;
		}

		printf("Parent got the message!\n");
	}

	return 0;
}
```

The *concurrency* of the system enables separate processes to be active at the same time, thus for the `write` and `read` to be transferring data through the pipe *at the same time*.
This simplifies our code as we don't need to worry about sending chunks of our data.

Note that we're `close`ing the end of the pipe that we aren't using in the corresponding processes.
Though the file descriptors are identical in each process following `fork`, each process does have a *separate* set of those descriptors.
Thus closing in one, doesn't impact the other.
Remember, processes provide *isolation*!

### The Shell

We can start to understand part of how to a shell might be implemented now!

**Setting up pipes.**
Lets start with the more obvious: for each `|` in a command, the shell will create a new `pipe`.
It is a little less obvious to understand how the standard output for one process is hooked up through a `pipe` to the standard input of the next process.
To do this, the shell does the following procedure:

1. Create a `pipe`.
2. `fork` the processes (a `fork` for each process in a pipeline).
3. In the *upstream* process `close(STDOUT_FILENO)`, and `dup2` the writable file descriptor in the pipe into `STDOUT_FILENO`.
4. In the *downstream* process `close(STDIN_FILENO)`, and `dup2` the readable file descriptor in the pipe into `STDIN_FILENO`.

Due to this *careful* usage of `close` to get rid of the old standard in/out, and `dup` or `dup2` to methodically replace it with the pipe, we can see how the shell sets up the processes in a pipeline!

Lets go over an example of setting up the file descriptors for a child process.
This does *not* set up the pipe-based communication between *two children*, so is not sufficient for a shell; but it is well on the way.
Pipes contain arbitrary streams of bytes, not just characters.
This example will

1. setup the input and output of two files to communicate over a pipe, and
2. send and receive binary data between processes.

```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void
perror_exit(char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

int
main(void)
{
	int fds[2];
	pid_t pid;

	/* make the pipe before we fork, so we can acccess it in each process */
	if (pipe(fds) == -1) perror_exit("Opening pipe");

	pid = fork();
	if (pid == -1) perror_exit("Forking process");

	if (pid == 0) {       /* child */
		/* Same as above, but for standard output */
		close(STDOUT_FILENO);
		if (dup2(fds[1], STDOUT_FILENO) == -1) perror_exit("child dup stdout");

		close(fds[0]);
		close(fds[1]);

		printf("%d %c %x", 42, '+', 42);
		fflush(stdout); /* make sure that we output to the stdout */

		exit(EXIT_SUCCESS);
	} else {              /* parent */
		int a, c;
		char b;

		/* close standard in... */
		close(STDIN_FILENO);
		/* ...and replace it with the input side of the pipe */
		if (dup2(fds[0], STDIN_FILENO) == -1) perror_exit("parent dup stdin");
		/*
		 * if we don't close the pipes, the child will
		 * always wait for additional input
		 */
		close(fds[0]);
		close(fds[1]);

		scanf("%d %c %x", &a, &b, &c);

		printf("%d %c %x", a, b, c);
		if (wait(NULL) == -1) perror_exit("parent's wait");
	}

	return 0;
}
```

**Closing pipes.**
`read`ing from a pipe will return that there is no more data on the pipe (i.e. return `0`) *only if all `write`-ends of the pipe are `close`d*.
This makes sense because we think of a pipe as a potentially infinite stream of bytes, thus the only way the system can know that there are no more bytes to be `read`, is if the `write` end of the pipe cannot receive more data, i.e. if it is `close`d.

This seems simple, in principle, but when implementing a shell, you use `dup` to make multiple copies of the `write` file descriptor.
In this case, the shell must be very careful to close its own copies because *if any `write` end of a pipe is open, the reader will not realize when there is no more data left*.
If you implement a shell, and it seems like commands are hanging, and not `exit`ing, this is likely why.

![A graphical sequence for the above code. Each image is a snapshot of the system after an operation is preformed. The red portions of each figure show what is changed in each operation. The parent creates a pipe, forks a child, which inherits a copy of all of the file descriptors (including the pipe), the parent and child close their stdin and stdout, respectively, the pipes descriptors are `dup`ed into those now vacant descriptors, and the pipe descriptors are closed. Now all processes are in a state where they can use their stdin/stdout/stderr descriptors as normal (`scanf`, `printf`), but the standard output from the child will get piped to the standard input of the parent. Shells do a similar set of operations, but in which a child has their standard output piped to the standard input of another child.](figures/pipe_proc.png)

**Question.**
If we wanted to have a parent, shell process setup two child connected by `|`, what would the *final* picture (in the image of the pictures above) look like?

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

**Question**: Again, track and explain the control flow through this program.

`SIGKILL` and `SIGSTOP` are unique in that they *cannot be disabled*, and handlers for them cannot be defined.
They enable non-optional control of a child by the parent.


### Parent/Child Coordination with Signals

Another example of coordination between parent and child processes.
We can use signals to get a notification that *a child has exited*!
Additionally, we can send the `SIGTERM` signal to terminate a process (this is used to implement the `kill` command line program -- see `man 1 kill`).

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> /* kill, pause */
#include <assert.h>
#include <sys/wait.h>

void
sig_handler(int signal_number, siginfo_t *info, void *context)
{
	switch(signal_number) {
	case SIGCHLD: {
		/* see documentation on `siginfo_t` in `man sigaction` */
		printf("%d: Child process %d has exited.\n", getpid(), info->si_pid);
		fflush(stdout);
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
setup_signal(int signo, void (*fn)(int , siginfo_t *, void *))
{
	sigset_t masked;
	struct sigaction siginfo;
	int ret;

	sigemptyset(&masked);
	sigaddset(&masked, signo);
	siginfo = (struct sigaction) {
		.sa_sigaction = fn,
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
	int status;

	setup_signal(SIGCHLD, sig_handler);
	setup_signal(SIGTERM, sig_handler);

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
		pause(); /* stop execution, wake upon signal */

		exit(EXIT_SUCCESS);
	}

	printf("%d: Parent asking child (%d) to terminate\n", getpid(), pid);
	kill(pid, SIGTERM); /* send the child the TERM signal */

	/* Wait for the sigchild notification of child termination! */
	pause();

	/* this should return immediately because waited for sigchld! */
	assert(pid == wait(&status));
	assert(WIFEXITED(status));

	return 0;
}
```

*Note:* You want to run this a few times on your system to see the output.
The auto-execution scripts of the lectures might cause wonky effects here due to concurrency.

We now see a couple of new features:

- The `SIGCHLD` signal is activated in the parent when a child process exits.
- We can use the `kill` function to *send a signal* to a another process owned by the same user (e.g. `gparmer`).
- The `pause` call says to stop execution (to pause) until a signal is triggered.

**Question**: Please try and explain the control flow through this program.

A couple of additional important functions:

- `raise` will trigger a signal in the current process (it is effectively a `kill(getpid(), ...)`).
- `ualarm` will set a recurring `SIGALRM` signal.
    This can be quite useful if your program needs to keep track of time in some way.

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
