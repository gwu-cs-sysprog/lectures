

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

**Question**: Track and explain the control flow through this program.

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

![A high-level overview of the control flow between parent and child in this code.](./figures/sig_chld.png)

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

Lets see the explicit interaction with between the slow call `wait`, and the signal handler:
```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

void
sig_handler(int signal_number, siginfo_t *info, void *context)
{
	printf("Alarm signal handler\n");

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
		/*
		 * We didn't set RESTART so that wait returns
		 * when a signal happens.
		 */
		.sa_flags     = SA_SIGINFO /* | SA_RESTART */
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

	setup_signal(SIGALRM);

	/* lets spin up a simple child */
	if ((pid = fork()) == 0) {
		pause(); /* await signal */
		exit(EXIT_SUCCESS);
	}
	/* our signal handler for the alarm should execute in a second */
	alarm(1);
	while (1) {
		pid_t ret;

		ret = wait(NULL);
		if (ret == -1) {
			if (errno == EINTR) {
				printf("Wait call interrupted by signal!\n");
				kill(pid, SIGTERM); /* goodbye sweet child! */
				/* loop again and `wait` */
			} else if (errno == ECHILD) {
				printf("Child has exited.\n");
				return 0; /* no more children! */
			}
		}
	}
}
```

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
