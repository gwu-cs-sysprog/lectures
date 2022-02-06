
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

## Files & Shared Memory

## Named Pipes

FIFOs are like pipes in that one process can write into the FIFO, and another process can read from it.
However, unlike pipes, FIFOs appear in the filesystem along side files.
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

## UNIX Domain Sockets

- `socket`
- `listen`
- `accept`

## Event Multiplexing

- `poll`
