
## Assorted Exercises

In the following, if you aren't *positive* of the answer, please run the program!
Note that we're omitting error checking in these programs to keep them terse.
Remember that in your programs, you must check and react to all errors.
This will require you to use the `man` pages to look up the required `#include`s.
If the output doesn't match a high-level intuition, how would you modify the program to match the intuition?
What are all of the potential output of the following programs?
Why?

### `fork` and Stream Behavior

``` c
fork();
fork();
printf("z");
```

``` c
printf("z");
fork();
fork();
```

``` c
printf("z");
fork();
write(STDOUT_FILENO, ".", 1);
```

### Wait Behavior

``` c
pid_t child;
int i;
int wait_param = 0;     /* or WNOHANG */
int output_w_write = 0; /* or 1 */

for (i = 0; i < 2; i++) {
	child = fork();
	if (child == 0) {
		sleep(1);
		if (output_w_write) write(STDOUT_FILENO, ".\n", 2);
		else                printf(".\n");
		exit(EXIT_SUCCESS);
	}
	waitpid(child, NULL, wait_param);
	write(STDOUT_FILENO, "Post-fork\n", 10);
}
/* ...are we done here? */
```

- What if change `wait_param` to equal `WNOHANG`?
- What if we change `output_w_write` to `1`?
- Why do we need to reap children?
    Why would this actually matter?

### `read` Behavior

``` c
pid_t child;
int fds[2];
char *msg = "What type of doggo is Penny?\n";

pipe(fds);

if ((child = fork()) == 0) {
	/* recall: `cat` reads its stdin, and outputs it to stdout */
	char *args[] = {"cat", NULL};

	close(STDIN_FILENO);
	dup2(fds[0], STDIN_FILENO);
	execvp(args[0], args);
}
write(fds[1], msg, sizeof(msg));
wait(NULL);
printf("100% good girl.");
```

- There are *multiple* bugs here.
    Find them and squish 'em.

### Signals

What is the following doing?

``` c
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h> /* sigaction and SIG* */

void
bt(void)
{
	void *bt[128];
	char **symbs;
	int nfns, i;

	nfns = backtrace(bt, 128);
	symbs = backtrace_symbols(bt, nfns);
	for (i = 0; i < nfns; i++) {
		printf("%s\n", symbs[i]);
	}
	free(symbs);
}

void
bar(int *val)
{
	*val = 42;
	bt();
}

void
foo(int *val)
{
	bar(val);
}

void
sig_fault_handler(int signal_number, siginfo_t *info, void *context)
{
	printf("Fault triggered at address %p.\n", info->si_addr);
	bt();
	exit(EXIT_FAILURE);

	return;
}

int
main(void)
{
	sigset_t masked;
	struct sigaction siginfo;
	int ret;
	int val;

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

	foo(&val);
	printf("---\nMoving on...\n---\n");
	fflush(stdout);
	foo(NULL);

	return 0;
}
```

- What are the various aspects of the output?
    What do the numbers mean?
	What do the strings mean?
- Explain what is happening *before* the "Moving on...".
- Explain what is happening *after* the "Moving on...".
- There is a lot of information there (lines being output) that aren't useful to the programmer.
    Make the output more useful by printing only the parts of the output that a programmer would find useful.
