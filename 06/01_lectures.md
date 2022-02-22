
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

- **Transparent IPC.**
    Do applications need to be changed at all to perform IPC?
	If not, they are *transparently* leveraging IPC.
	Shell-driven pipes have a huge benefit that they enable transparent IPC!
- **Named IPC.**
    If we want multiple process to communicate that can't rely on a comment parent to coordinate that communication, they need a means to find an "name" the IPC mechanism.
	Named IPC is in conflict with transparent IPC as we likely need to use a specific API to access the IPC mechanism.
- **Channel-based IPC.**
    Often we want to send *messages* between processes such that a sent message, when received is removed from the channel -- once read, it won't be read again.
	This enables processes to receive some request, do some processing on it, then move on to the next request.
- **Multi-client communication.**
    We often want to create a process that can provide services to *multiple* other "client" processes.
	Clients request service, and the "server" process receives these requests, and provides replies.

Lets assess `pipe`s in this taxonomy:

- ✓ **Transparent IPC.** - Pipes are pervasive for a reason!
    They enable composition of programs via pipelines despite the programs not even knowing they are in the pipeline!
- ✗ **Named IPC.** - Na.
    The parent has to set it all up -- lots of `dup`, `close`, and `pipe`.
- ✓ **Channel-based IPC.** - Data written to a pipe is removed by the reader.
- ✗ **Multi-client communication.** - Pipes are really there to pass a stream of data from one process to the other.

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
Thus, they are not useful for channel-based communication between multiple processes.

On the other side, files have a very  *useful* properti that we'd like to use in a good solution to IPC: they *have a location in the FS* that the communicating processes can both use to find the file, thus avoiding the need for a shared parent.


- ✗ **Transparent IPC** - We have to open the file explicitly.
- ✓ **Named IPC.** - Each file has a path in the filesystem.
- ✗ **Channel-based IPC.** - When we read out of a file, the data remains, making it hard to know what we've read, and what is yet to be written.
    A stream of messages placed into a file also will expend your disk!
- ✗ **Multi-client communication.** - It isn't clear how multiple clients can convey separate requests, and can receive separate replies from a server.

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

Regardless, we do see named pipes as a cool option: named pipes enable us to use the filesystem to identify the pipe used for IPC.
This enables communicating processes without shared parents to leverage IPC.
This enables pipes to live up to the UNIX motto: *everything is a file*.

### Challenges with Named Pipes for Multi-Client Communication

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

![Using two named pipes, one per communication direction, to communicate between a server (right) and clients (left). The color of the arrows designates the client for whom the communication is relevant. Note that on the way back to the clients, we "lose" track of which client each reply is for. This simply isn't tracked with named pipes.](./figures/fifo.png)

**Named pipes summary.**
These solve an important problem: how can we have multiple processes find a "pipe" to use for communication even if they don't have a parent to coordinate that communication?
They use a filesystem path/name to identify the pipe.
However, they are not suitable for a single processes (a server) to communicate with multiple clients as they don't enable the communication for each client to be separated in the server.

- ✗ **Transparent IPC.** - we have to use the `mkfifo` API explicitly.
- ✓ **Named IPC.** - the named pipe is represented as a file in the filesystem.
- ✓ **Channel-based IPC.** - read data is removed from the pipe.
- ✗ **Multi-client communication.** - a server cannot tell the difference between clients, nor can they send responses to specific clients.

## UNIX Domain Sockets

*Sockets* are the mechanism provided by UNIX to communicate over the *network*!
However, they can also be used to communicate between processes on your system through *UNIX domain sockets*.

A few key concepts for domain sockets:

1. They are presented in the filesystem as a file, similar to named pipes.
    This enables them to be accessed by multiple communicating processes that don't necessarily share a parent to coordinate that communication.
2. Each new *client* that attempts to connect to a *server* will be represented as a *separate descriptor* in the server, thus enabling the server to separate its communication with on, from the other.
    The goal is to create a descriptor for *each pair of communicating processes*.
3. Each descriptor to a domain socket is bi-directional -- it can be `read` and `write`n to, thus making communication back and forth quite a bit easier.

### Domain sockets for Multi-Client Communication

Lets look at an example were we want a server to receive a client's requests as strings, and to reply with those same strings.
This isn't useful, per-say, but demonstrates how this communication can happen.
Notably, we want to enable the server to communicate with different clients!

- A *server* receives IPC requests from *clients*.
    Note that this is similar to how a server on the internet serves webpages to multiple clients (and, indeed, the code is similar!).
- The server's functions include `socket`, `bind`, and `listen`.
    `socket` creates a domain socket file descriptor
- The server creates a *separate descriptor* for each client using `accept`.
- The client's functions include `socket` and `connect`.

Most of these functions are complex and have tons of options.
Most of them have been distilled into the following functions in `06/domain_sockets.h`:

- `int domain_socket_server_create(const char *file_name)` - Create a descriptor to the "server" end of the IPC channel identified by `file_name` in the filesystem, similar to the named pipes before.
- `int domain_socket_client_create(const char *file_name)` - Create a descriptor to the "client" end of the IPC channel identified by a file name.
    One constraint is that the *server must create the domain socket first*, or else this call (the `connect`) will fail.

The server's descriptor is not meant to be used for direct communication (i.e. should not be used for `read`, and `write`).
Instead, it is used to *create new descriptors, one per client*!
With a descriptor per-client, we have the fortunate ability to communicate explicitly with each client without the same problem of messages getting messed up in named pipes.

![A sequence of using domain sockets to communicate between multiple clients and a server. (a) The single domain socket that clients can attempt to `connect` to. (b) The server using `accept` to create a new descriptor and channel for communication with the red client (thus the "red" channel). (c) Subsequent communication with that client is explicitly through that channel, so the server can send specifically to that client. (d) The server creates a *separate* channel for communication with the blue client.](./figures/domain_sockets.png)

#### Setting up Domain Sockets

Two functions that both take an argument which is the domain socket name/path in the file system, and return a descriptor to the socket.
For the most part, you can just use these functions in your code directly by using `06/domain_sockets.h`.

```c
#include "06/domain_sockets.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

void
unlink_domain_socket(int status, void *filename)
{
	unlink(filename);
	free(filename);
}

#define MAX_BUF_SZ 128

void
server(int num_clients, char *filename)
{
	char buf[MAX_BUF_SZ];
	int new_client, amnt, i, socket_desc;

	socket_desc = domain_socket_server_create(filename);
	if (socket_desc < 0) exit(EXIT_FAILURE); /* should do proper cleanup */
	on_exit(unlink_domain_socket, strdup(filename));

	/*
	 * Service `num_clients` number of clients, one at a time.F
	 * For many servers, this might be an infinite loop.
	 */
	for (i = 0; i < num_clients; i++) {
		/*
		 * We use this new descriptor to communicate with *this* client.
		 * This is the key function that enables us to create per-client
		 * descriptors. It only returns when a client is ready to communicate.
		 */
		if ((new_client = accept(socket_desc, NULL, NULL)) == -1) exit(EXIT_FAILURE);
		printf("Server: New client connected with new file descriptor %d.\n", new_client);
		fflush(stdout);

		amnt = read(new_client, buf, MAX_BUF_SZ - 1);
		if (amnt == -1) exit(EXIT_FAILURE);
		buf[amnt] = '\0'; /* ensure null termination of the string */
		printf("Server received message (sz %d): \"%s\". Replying!\n", amnt, buf);
		fflush(stdout);

		/* send the client a reply */
		if (write(new_client, buf, amnt) < 0) exit(EXIT_FAILURE);
		/* Done with communication with this client */
		close(new_client);
	}
	close(socket_desc);

	exit(EXIT_SUCCESS);
}

void
client(char *filename)
{
	char msg[MAX_BUF_SZ];
	int  amnt = 0, socket_desc;

	socket_desc = domain_socket_client_create(filename);
	if (socket_desc < 0) exit(EXIT_FAILURE);
	printf("1. Client %d connected to server.\n", getpid());
	fflush(stdout);

	snprintf(msg, MAX_BUF_SZ - 1, "Citizen %d: Penny for Pawsident!", getpid());
	amnt = write(socket_desc, msg, strlen(msg) + 1);
	if (amnt < 0) exit(EXIT_FAILURE);
	printf("2. Client %d request sent message to server.\n", getpid());
	fflush(stdout);

	if (read(socket_desc, msg, amnt) < 0) exit(EXIT_FAILURE);
	msg[amnt] = '\0';
	printf("3. Client %d reply received from server: %s\n", getpid(), msg);
	fflush(stdout);

	close(socket_desc);

	exit(EXIT_SUCCESS);
}

int
main(void)
{
	char *channel_name = "pennys_channel";
	int nclients = 2;
	int i;

	if (fork() == 0) server(nclients, channel_name);
	/* wait for the server to create the domain socket */
	sleep(1);
	for (i = 0; i < nclients; i++) {
		if (fork() == 0) client(channel_name);
	}
	/* wait for all of the children */
	while (wait(NULL) != -1);

	return 0;
}
```

The server's call to `accept` is the key difference of domain sockets from named pipes.
It enables us to have per-client descriptors we can use to separately communicate (via `read`/`write`) to each client.

- ✗ **Transparent IPC.** - We have to explicitly use the socket APIs.
- ✓ **Named IPC.** - Uses a file name to name the domain socket.
- ✓ **Channel-based IPC.** - When data is read, it is removed from the socket.
- ✓ **Multi-client communication.** - The server separates each client's communication into separate descriptors.

> Aside:
> Sockets are the main way to communicate over the network (i.e. to chat with the Internet).
> The APIs you'd use to create network sockets are the same, it simply requires setting up the `socket` and the `bind`ing of the socket in a network-specific manner.
