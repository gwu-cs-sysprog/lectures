
## Revisiting IPC with Multiple Clients

### System Services

The `systemctl` command enables you to understand many of the services on the system (of which there are many: `systemctl --list-units | wc -l` yields `244` on my system).

```
$ systemctl --list-units
...
  cups.service           loaded active     running   CUPS Scheduler
...
  gdm.service            loaded active     running   GNOME Display Manager
...
  ssh.service            loaded active     running   OpenBSD Secure Shell server
...
  NetworkManager.service loaded active     running   Network Manager
...
  openvpn.service        loaded active     exited    OpenVPN service
...
```

I've pulled out a few selections that are easier to relate to:

- CUPS which is the service used to receive print jobs.
- [GDM](https://en.wikipedia.org/wiki/GNOME_Display_Manager) which manages your *graphical logins* on Ubuntu.
- OpenSSH which manages your *remote* logins through `ssh`.
- NetworkManager that you interact with when you select which wifi hotspot to use.
- OpenVPN which handles your VPN logins to the school network.
    You can see that the service is currently "exited" as I'm not running VPN now.

Each of these services communicates with many clients using domain sockets.

### Understanding Descriptor Events with `poll`

Each of these services is a process that any client send requests to.
We've seen that domain sockets can help us to talk to many different clients as each is represented with a separate file descriptor.
How does the service process know *which of the file descriptors* has information available on it?
Imagine the following case:

1. a client, *A*, connects to a service.
2. a client, *B*, connects to the same service.
3. *B* immediately sends a request, while *A* goes into a `sleep(100)` (or, more realistically, simply does some expensive computation).

If the server issues a `read` on the descriptor for *A*, it will block for 100 seconds!
Worse, it won't service *B* despite it making a request immediately.
Why are we waiting for a slow client when there is a fast client with data already available?
Yikes.

We'd really like a facility that can tell us *which* descriptor's have data and are ready to be read from, and which are ready to be written to!
Luckily, UNIX comes to the rescue with its *event notification* APIs.
These are APIs that let us understand when a file descriptor has an *event* (i.e. a client writes to it) and is now readable or writable.
These include three functions: `poll`, `select`, and (the more modern) `epoll`.
Lets look at how to use `poll`!

### Event Loops and `poll`

Lets look at some pseudocode for using `poll`.

```python
fdinfo[NUM_FDS] = {
	# initialized to all fds of interest, listening for read and write events
}
while True:
	poll(fdinfo, NUM_FDS, -1) # -1 = no timeout
	for fdi in fdinfo:
		# check and respond to each of the possible events
		if fdi.revents & (POLLHUP | POLLERR):
			# process closed fds
		if fdi.revents & POLLIN:
			# read off of, or accept on the file desciptor
		if fdi.revents & POLLOUT:
			# write to the file desciptor
```

This yields what is called an *event loop* -- we loop, each time processing a single event.
You see event loops in most GUIs (where each event is a key/mouse press), and in web programming where javascript callbacks are executed by the browser's event loop.
Importantly, *we only process descriptors that have events, thus can avoid blocking on descriptors that don't have available data!*
This solves our previous problem: a server won't block awaiting communication with a client that is delayed, or never writes to a channel, instead only `read`ing/`write`ing to descriptors that are ready.

### `poll` API

- `int poll(struct pollfd *fds, nfds_t nfds, int timeout)` - We pass in an array of `struct pollfd`s of length `nfds`, with each entry corresponding to a single file descriptor we want to get information about.
    The `timeout` is in milliseconds, and enables `poll` to return after that amount of time returns even if none of the file descriptors has an event.
	A negative `timeout` is interpreted as "infinite", while `0` means that `poll` will return immediately with any current events.

Lets check out the `struct pollfd`:

``` c
struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
```

When we make the `poll` call, we populate the `fd` and `events` fields with the file descriptors we're interested in, and which events we're interested in retrieving.
`events` is a "bitmap" which means each bit in the value denotes a different type of event, and we bitwise *or* event types together when writing them into `events`.
These event types include:

- `POLLIN` - is there data available to be `read`?
    If the file descriptor is the domain socket that we use for `accept`, then this `POLLIN` means that a new client request is ready to `accept`.
- `POLLOUT` - is there data available to be `write`n?

We can almost always set `events = POLLIN | POLLOUT` as we wait to wait for both.

When `poll` returns, we determine which events happened by looking at the contents of the `revents` field.
In addition to `POLLIN` and `POLLOUT` which tell us if data is ready to be `read` or `written`, we have the following:

- `POLLHUP` - The other side of the pipe closed its descriptor!
    Subsequent `read`s to the descriptor will return `0`.
	We can likely `close` our descriptor.
- `POLLERR` - Again, there was some sort of a problem with the descriptor, and we should likely `close` it, terminating communication.

### Example `poll` Code

Lets put this all together.
For simplicity, in this example, we'll assume that we can always `write` to a descriptor without blocking.
This isn't generally true if you're writing large amounts of data, and in "real code" you'd also want to handle `POLLOUT` appropriately.

``` c
#include "06/domain_sockets.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include <poll.h>

void
panic(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void
client(char *filename)
{
	int i, socket_desc;

	sleep(1); /* await the domain socket creation by the server */
	socket_desc = domain_socket_client_create(filename);
	if (socket_desc < 0) {
		perror("domain socket client create");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < 5; i++) {
		char b;
		if (write(socket_desc, ".", 1) == -1) panic("client write");
		if (read(socket_desc, &b, 1) == -1)   panic("client read");
		printf("c %d: %c\n", getpid(), b);
	}

	close(socket_desc);
	exit(EXIT_SUCCESS);
}

/* we can track max 16 fds */
#define MAX_FDS 16

void
server(char *filename)
{
	int socket_desc, num_fds = 0;
	struct pollfd poll_fds[MAX_FDS];

	/*** Initialize the domain socket and its pollfd ***/
	socket_desc = domain_socket_server_create(filename);
	if (socket_desc < 0) {
		unlink(filename); /* remove the previous domain socket file if it exists */
		socket_desc = domain_socket_server_create(filename);
		if (socket_desc < 0) panic("server domain socket creation");
	}

	/* Initialize all pollfd structs to 0 */
	memset(poll_fds, 0, sizeof(struct pollfd) * MAX_FDS);
	poll_fds[0] = (struct pollfd) {
		.fd     = socket_desc,
		.events = POLLIN,
	};
	num_fds++;

	/*** The event loop ***/
	while (1) {
		int ret, new_client, i;

		/*** Poll; if we don't get a client for a second, exit ***/
		ret = poll(poll_fds, num_fds, 1000);
		if (ret == -1) panic("poll error");
		/*
		 * If we timeout, break out of the loop.
		 * This isn't what you'd normally do as servers stick around!
		 */
		if (ret == 0) break;

		/*** Accept file descriptor has a new client connecting! ***/
		if (poll_fds[0].revents & POLLIN) {
			if ((new_client = accept(socket_desc, NULL, NULL)) == -1) panic("server accept");
			/* add a new file descriptor! */
			poll_fds[num_fds] = (struct pollfd) {
				.fd = new_client,
				.events = POLLIN
			};
			num_fds++;
			poll_fds[0].revents = 0;
			printf("server: created client connection %d\n", new_client);
		}
		/*** Communicate with clients! ***/
		for (i = 1; i < num_fds; i++) {
			if (poll_fds[i].revents & (POLLHUP | POLLERR)) {
				printf("server: closing client connection %d\n", poll_fds[i].fd);
				poll_fds[i].revents = 0;
				close(poll_fds[i].fd);
				/* replace the fd to fill the gap */
				poll_fds[i] = poll_fds[num_fds - 1];
				num_fds--;
				/* make sure to check the fd we used to fill the gap */
				i--;

				continue;
			}
			if (poll_fds[i].revents & POLLIN) {
				char b;

				poll_fds[i].revents = 0;
				/* our server is simply sending a '*' for each input character */
				if (read(poll_fds[i].fd, &b, 1) == -1) panic("server read");
				if (write(poll_fds[i].fd, "*", 1) == -1) panic("server write");
			}
		}
	}
	close(socket_desc);

	exit(EXIT_SUCCESS);
}

int
main(void)
{
	char *ds = "domain_socket";

	if (fork() == 0) client(ds);
	if (fork() == 0) client(ds);
	server(ds);

	return 0;
}
```
