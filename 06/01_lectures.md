
# I/O: Inter-Process Communication

We've seen that processes often coordinate through pipes, signals, and `wait`.
This is necessary when coordinating between parents and children in shells (both on the terminal, and GUIs).
These are useful when children have a common parent (thus can share pipes), and where the parent knows that the children want to communicate before they are `exec`ed.
Modern systems require a lot of flexibility than this.
Generally, we want a process to be able to decide who to coordinate with as they execute given their own goals.
For example, the shell -- when it executes a child -- doesn't know if it wants to have a GUI, thus needs to talk to the window server.

We require more dynamic, more flexible forms of coordination between processes.
Generally, we call this coordination *Inter-Process Communication* or *IPC*.
*When do we generally want more flexible forms of IPC* than pipes, signals, and wait?

1. If the parent doesn't know what other processes with which a child wants IPC.
2. If the parent didn't create the other process with which a child wants IPC.
3. The process with which we want IPC belongs to a different user.
4. The process with which we want IPC has access to special resources in the system (e.g. graphics cards, other hardware, or filesystem data).

Even applications that are implemented as many processes often require IPC between those processes.

## IPC Example: Modern Browsers

Browsers all require a *lot* of IPC.
Each tab is a separate process, and there is a single *browser management* process that manages the GUI and other browser resources.
This is motivated by security concerns: a tab that fails or is compromised by an attacker cannot access the data of another tab!
In the early (first 15-20 years) of browsers, this wasn't the case, and you had to be careful about opening your banking webpage next to webpages that were suspect.
The browser management process communicates with each of the child tab processes to send them user input (mouse clicks, keyboard input), and receive a bitmap to display on screen.
Lets see this structure:

```
$ ps aux | grep firefox
... 8029 ... /usr/lib/firefox/firefox -new-window
... 8151 ... /usr/lib/firefox/firefox -contentproc -childID 1 -isForBrowser ... 8029 ...
...
```

I have an embarrassing number of tabs open, so I've taken the liberty of manually filtering this output.
We can see that the process with pid `8029` is the parent, browser management process, and the process `8151` is a child managing a tab (note, you can see processes arranges as a process  tree using `ps axjf`).
There are many other children processes managing tabs.

Lets try and figure out how they communicate with each other.
First, lets look at the file descriptors of the tab's processes (the child).

```
$ ls -l /proc/8151/fd/
0 -> /dev/null
1 -> socket:[100774]
...
11 -> /memfd:mozilla-ipc
4 -> socket:[107464]
5 -> socket:[107467]
7 -> pipe:[110438]
```

We see that there is no standard input (`/dev/null` is "nothing"), and that there are a few means of IPC:

- `pipe` - We know these!
- `memfd` - A Linux-specific way of sharing a range of memory between the processes.
    Stores to the memory can be seen by all sharing processes.
- `socket` - Another pipe-line channel in which bytes written into the socket can be read out on the other side.
    A key difference is that socket connections between processes can be created without requiring the inheritance of descriptors via `fork`.

Lets look at the browser management process, and see what's going on.
Notably, we want to see how the tab's processes communicate with it.
Thus, lets look at the descriptors for the parent:

```
$ ls -l /proc/8029/fd/ | awk '{print $9 $10 $11}'
110 -> /home/ycombinator/.mozilla/.../https+++mail.google.com...data.sqlite
171 -> /home/ycombinator/.mozilla/.../https+++www.youtube.com....data.sqlite
123 -> /home/ycombinator/.mozilla/.../formhistory.sqlite
130 -> /home/ycombinator/.mozilla/.../bookmarks.sqlite
58 -> /home/ycombinator/.mozilla/.../cookies.sqlite
3 -> /dev/dri/renderD128
43 -> /dev/dri/card0
...
100 -> /memfd:mozilla-ipc
60 -> socket:[107464]
64 -> socket:[107467]
82 -> pipe:[110438]
```

This is *heavily filtered*.
The first, highlighted section shows that the browser management process uses a database (`sqlite`) to access a webpage's data (see `gmail` and `youtube`), store form histories, bookmarks, and cookies.
It also uses the `dri` device is part of the [Direct Rendering Infrastructure](https://en.wikipedia.org/wiki/Direct_Rendering_Infrastructure) which enables it to communicate with the X Window Server, and display what we see on screen.
Finally, we see it uses the same means of IPC as the client, and if we carefully look at the `[ids]` of the sockets and pipes, we can see that they match those in the child tab process!
We can see that the tab has multiple IPC channels open with the parent, and can access them with file descriptors.

## Goals of IPC Mechanisms

There are multiple IPC mechanisms in the system, and they all represent different trade-offs.
They might be good for some things, and bad at others.

- [ ] **Transparent IPC.**
    Do applications need to be changed at all to perform IPC?
	If not, they are *transparently* leveraging IPC.
- [ ] **Simple IPC setup code.**
    Though this is subjective, how many hoops do we need to jump through to setup the IPC code.
	If IPC is transparent, then there is *zero* setup, which is, of course, quite simple.
- [ ] **Named IPC.**
    If we want multiple process that are not related by a common parent to communicate, they need a means to find an "name" the IPC mechanism.
	Named IPC is in conflict with transparent IPC.
- [ ] **Channel-based IPC.**
    Often we want to send *messages* between processes.
	You can think of messages as being similar to the arguments and return values we pass to functions.
	It is natural that we'd want to pass arguments to a
- [ ] **Multi-client communication.**
    We often want to create a process that can provide services to *multiple* other "client" processes.
	Clients request service, and the "server" process receives these requests, and provides replies.

Lets assess `pipe`s in this taxonomy:

- [x] **Transparent IPC.**
- [x] **Simple IPC setup code.**
- [ ] **Named IPC.**
- [x] **Channel-based IPC.**
- [ ] **Multi-client communication.**

## Files & Shared Memory

If the goal is to send data from one process to another, one option is found in the filesystem (FS): can't we just use files to share?
Toward this, we saw in the section on FS I/O that we can open a file, and `read` and `write` (or `fread`/`fwrite`) from it from *multiple* processes.
This will certainly get data from one process to the other.
However, it has a number of shortcomings:

1. If you want to pass a potentially infinite stream of data between processes (think `pipe`s), then we'd end up with an infinite file.
    That translates to your disk running out of space very quickly, with little benefit.
	Take-away: files are for a finite amount of data, not for an infinite stream of data.
2. Given this, processes must assume that the file has a specific format, and they have indication of if the other process has accessed/read data in the file.
    An example of a specific format: maybe the file is treated as a single string of a constant length.
3. There isn't any indication about when other processes have modified the file, thus the processes might overwrite each other's data^[This is called a "race condition". File "locking" helps solve this issue, but we'll delay discussing locking until the OS class.].

To emphasize these problems, lets try and implement a channel in a file to send data between processes.
We just want to send a simple string repetitively from one process to the other.

```c
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int
main(void)
{
	int ret;

	ret = creat("string_data", 0777);
	assert(ret != -1);

	if (fork() == 0) {
		char *msg[] = {"penny for pawsident", "america for good doggies"};
		int fd = open("string_data", O_WRONLY);
		assert(fd != -1);

		/* send the first message! */
		ret = write(fd, msg[0], strlen(msg[0]) + 1);
		assert(ret == (int)strlen(msg[0]) + 1);
		/* send the second message! */
		ret = lseek(fd, 0, SEEK_SET);
		ret = write(fd, msg[1], strlen(msg[1]) + 1);
		assert(ret == (int)strlen(msg[1]) + 1);

		close(fd);
		exit(EXIT_SUCCESS);
	} else {
		char data[32];
		int fd = open("string_data", O_RDONLY);
		assert(fd != -1);

		memset(data, 0, 32);
		ret = read(fd, data, 32);
		assert(ret != -1);
		printf("msg1: %s\n", data);

		ret = lseek(fd, 0, SEEK_SET);
		assert(ret != -1);
		ret = read(fd, data, 32);
		assert(ret != -1);
		printf("msg2: %s\n", data);
	}
	wait(NULL);
	unlink("string_data");

	return 0;
}
	```

You can see that there are some problems here.
If we run it many times, we can see that sometimes we don't see any messages, sometimes we only see the first, sometimes we only see the second, and other times combinations of all three options.

On the other side, files have a very  *useful* properti that we'd like to use in a good solution to IPC: they *have a location in the FS* that the communicating processes can both use to find the file, thus avoiding the need for a shared parent.

> An aside: you can use `mmap` to map a *file* into the address space, and if you map it `MAP_SHARED` (instead of `MAP_PRIVATE`), then the memory will be shared between processes.
> When one process does a store to the memory, the other process will see that store immediately!
> We can use this to pass data between processes, but we still have many of the problems above.
> How do we avoid conflicting modifications to the memory, get notifications that modifications have been made, and make sure that the data is formatted in an organized (finite) manner?

## Named Pipes

The core problem with files is that they aren't channels that remove existing data when it is "consumed" (`read`).
But they have the significant up-side that they have a "name" in the filesystem that multiple otherwise independent processes can use to access the communication medium.

Named pipes or FIFOs are like pipes in that one process can write into the FIFO, and another process can read from it.
Thus, unlike files, they have the desirable property of channels in which data read from the channel is consumed.
However, like files (and unlike pipes) they have a "name" in the filesystem -- FIFOs appear in the filesystem along-side files.
The `stat` function will let you know that a file is a FIFO if the `st_mode` is `S_IFIFO`.
Since these pipes appear in the filesystem, they are called **named pipes**.
Two processes that wish to communicate need only both know where in the filesystem they agree to find the named pipe.

Lets see an example of using named pipes:

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
proc1(void)
{
	int fd, ret;

	fd = open("office_hours", O_WRONLY);
	assert(fd != -1);

	ret = write(fd, "halp!", 5);
	assert(ret == 5);
	close(fd);
}

void
proc2(void)
{
	char msg[6];
	int fd, ret;

	memset(msg, 0, 6);
	fd = open("office_hours", O_RDONLY);
	assert(fd != -1);

	ret = read(fd, msg, 5);
	assert(ret == 5);
	close(fd);

	printf("What I hear at office hours is \"%s\".", msg);
	unlink("office_hours");
}

int
main(void)
{
	int ret;

	/* This is how we create a FIFO. A FIFO version of creat. */
	ret = mkfifo("office_hours", 0777);
	assert(ret == 0);

	if (fork() == 0) {
		proc1();
		exit(EXIT_SUCCESS);
	} else {
		proc2();
		wait(NULL);
	}

	return 0;
}
```

There is one very awkward named pipe behavior.
Processes attempting to `open` a named pipe will block (the `open` will not return) until processes have opened it separately as both *readable* and *writable*.
This is awkward because how would a process *know* when another process might want to communicate with it?
If a process gets IPC requests from many others (think the browser manager), then it doesn't want to block awaiting a new communication; it wants to service the requests of other processes.

Regardless, we do see a cool option: named pipes enable us to use the filesystem to identify the pipe used for IPC.
This enables communicating processes without shared parents to leverage IPC.
This enables pipes to live up to the UNIX motto: *everything is a file*.

### Challenge in Using Named Pipes for Multi-Client Communication

Lets check out an example that demonstrates how using named pipes for communication between a single process and multiple *clients* has a number of challenges.

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Receive requests, and send them immediately as a response.
 * You can imagine that interesting computation could be done
 * to formulate a response.
 */
void
instructor(void)
{
	int req, resp, ret;
	char msg[32];

	req = open("requests", O_RDONLY);
	assert(req != -1);
	resp = open("responses", O_WRONLY);
	assert(resp != -1);

	while (1) {
		ret = read(req, msg, 32);
		if (ret == 0) break;
		assert(ret != -1);
		ret = write(resp, msg, ret);
		if (ret == 0) break;
		assert(ret != -1);
	}

	close(req);
	close(resp);
}

/*
 * Make a "request" with our pid, and get a response,
 * hopefully also our pid.
 */
void
student(void)
{
	int req, resp, ret;
	char msg[32];

	req = open("requests", O_WRONLY);
	assert(req != -1);
	resp = open("responses", O_RDONLY);
	assert(resp != -1);

	ret = snprintf(msg, 32, "%d", getpid());
	ret = write(req, msg, ret);
	assert(ret != -1);
	ret = read(resp, msg, 32);
	assert(ret != -1);

	printf("%d: %s\n", getpid(), msg);

	close(req);
	close(resp);
}

void
close_fifos(void)
{
	unlink("requests");
	unlink("responses");
}

int
main(void)
{
	int ret, i;
	pid_t pids[3];

	/* clients write to this, server reads */
	ret = mkfifo("requests", 0777);
	assert(ret == 0);
	/* server sends replies to this, clients read */
	ret = mkfifo("responses", 0777);
	assert(ret == 0);

	/* create 1 instructor that is lecturing */
	if ((pids[0] = fork()) == 0) {
		instructor();
		return 0;
	}
	/* Create 2 students "listening" to the lecture */
	for (i = 0; i < 2; i++) {
		if ((pids[i + 1] = fork()) == 0) {
			student();
			return 0;
		}
	}

	atexit(close_fifos);
	sleep(1);
	for (i = 0; i < 3; i++) kill(pids[i], SIGTERM);
	while (wait(NULL) != -1);

	return 0;
}
```


If executed many times, you see the expected result:

```
167907: 167907
167908: 167908
```

...but also strange results:

```
167941: 167940167941
```

If this is executed many times, we see a few properties of named pipes.

1. There isn't really a way to predict which student sees which data -- this is determined by the concurrency of the system.
2. Sometimes a student is "starved" of data completely.
3. The instructor doesn't have any control over *which* student should receive which data.

## UNIX Domain Sockets

*Sockets* are the mechanism provided by UNIX to communicate over the *network*!
However, they can also be used to communicate between processes on your system through *UNIX domain sockets*.

There are a few interesting new concepts required for sockets.
Importantly, the goal is to create a descriptor for *each pair of communicating processes*.
This is essential so that the processes can be explicit about who they want to send data to and receive data from.

### Domain sockets for Multi-Client Communication

- A *server* receives IPC requests from *clients*.
    Note that this is similar to how a server on the internet serves webpages to multiple clients (and, indeed, the code is similar!).
- The server's functions include `socket`, `bind`, and `listen`.
    `socket` creates a domain socket file descriptor
- The server creates a *separate descriptor* for each client using `accept`.
- The client's functions include `socket` and `connect`.

``` c
int
domain_socket_server_create(const char *file_name)
{
	struct sockaddr_un addr;
	int fd;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, file_name, sizeof(addr.sun_path)-1);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind error");
		exit(-1);
	}

	if (listen(fd, 5) == -1) {
		perror("listen error");
		exit(-1);
	}

	return fd;
}
```

``` c
int
domain_socket_client_create(const char *file_name)
{
	struct sockaddr_un addr;
	int fd;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, file_name, sizeof(addr.sun_path)-1);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	return fd;
}
```

Lets see an example of connecting the client to the server:

```c
/*
 * The core code for domain sockets is in
 * - 06/domain_socket_client.c
 * - 06/domain_socket_server.c
 * This code just runs those, passing the domain socket
 * file name in the environment variables.
 */

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

int
main(void)
{
	char *dsname = "domain_socket_file";
	int ret;

	ret = setenv("DOMAIN_SOCKET_FILENAME", dsname, 1);
	assert(ret != -1);

	if (fork() == 0) {
		char *args[] = {"./06/domain_socket_client.bin", NULL};
		/* let the server run first and create the domain socket file */
		sleep(1);
		execvp(args[0], args);
	} else {
		char *args[] = {"./06/domain_socket_server.bin", NULL};
		execvp(args[0], args);
	}

	return 0;
}
```

- `socket`
- `listen`
- `accept`

## Event Multiplexing

- `poll`
