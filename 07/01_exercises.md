
# Reinforcing Ideas: Assorted Exercises and Event Notification

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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int
main(void)
{
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

	return 0;
}
```

- What if change `wait_param` to equal `WNOHANG`?
- What if we change `output_w_write` to `1`?
- Why do we need to reap children?
    Why would this actually matter?

### `read` Behavior

``` c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int
main(void)
{
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
	write(fds[1], msg, strlen(msg));
	printf("100%% good girl.");
	wait(NULL);

	return 0;
}
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

### Communication with Multiple Clients

Communicating with multiple clients is hard.
Domain sockets are complicated, but there are challenges around blocking on `read`s.
What if one client is very "slow", and we block waiting for them?

``` c
#include "06/domain_sockets.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>

void
panic(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void
client(char *filename, int slowdown)
{
	int i, socket_desc;
	char b;

	socket_desc = domain_socket_client_create(filename);
	if (socket_desc < 0) {
		perror("domain socket client create");
		exit(EXIT_FAILURE);
	}

	/* delay after creating connection, but before communicating */
	sleep(slowdown);
	if (write(socket_desc, ".", 1) == -1) panic("client write");
	if (read(socket_desc, &b, 1) == -1)   panic("client read");
	printf("c: %c\n", b);

	close(socket_desc);
	exit(EXIT_SUCCESS);
}

void
client_slow(char *filename)
{
	client(filename, 3);
}

void
client_fast(char *filename)
{
	client(filename, 1);
}

int
main(void)
{
	char *ds = "domain_socket";
	int socket_desc, i;

	socket_desc = domain_socket_server_create(ds);
	if (socket_desc < 0) {
		/* remove the previous domain socket file if it exists */
		unlink(ds);
		socket_desc = domain_socket_server_create(ds);
		if (socket_desc < 0) panic("server domain socket creation");
	}

	/* TODO: change this order. What changes? */
	if (fork() == 0) client_slow(ds);
	if (fork() == 0) client_fast(ds);

	/* handle two clients, one after the other */
	for (i = 0; i < 2; i++) {
		int ret, new_client, i;
		char b;

		new_client = accept(socket_desc, NULL, NULL);
		if (new_client == -1) panic("server accept");

		/* read from, then write to the client! */
		if (read(new_client, &b, 1) == -1)   panic("server read");
		if (write(new_client, "*", 1) == -1) panic("server write");
		close(new_client);
	}

	close(socket_desc);
	/* reap all children */
	while (wait(NULL) != -1) ;

	return 0;
}
```

- See the TODO in the `main`.
    When you change the order, what do you see?
- Is there generally a problem when we use both fast and slow clients?
