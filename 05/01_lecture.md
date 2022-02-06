
# Filesystem Input/Output

We've seen how to use `pipe`s for communication between processes, and how descriptors are used to interact with system resources.
In this module, we'll dive into how to access the file system and the files and directories on your system!

## Accessing File Contents

The filesystem is one of the core abstractions on UNIX systems and indeed, on modern OSes.
Each file is identified by a path through directories.
Interestingly, an early goal of UNIX is that "everything is a file".
This is a strange statement as it raises the question "what wouldn't normally be a file?"
Some examples:

- We know of processes as executable instances of programs so certainly they cannot be files, right?
    We've seen that they are represented by files in `/proc/*`!
	These files are used to provide "task monitor" type functionality (showing running processes) and `gdb`.
- Devices are the physical parts of our computers like screens, keyboards, USB devices, hard-drives, and networks.
    They are all represented by files in `/dev/*`!
	You can actually `cat` a file directly to your disk^[Please, please do *not* try this.]!
- Want random values? `/dev/random`.
    Want the complete absence of anything? `/dev/null`^[This is useful to redirect shell output to if you don't care about the output.].
	Want to update your power settings? There are files in `/sys/*` for that.

When everything is a file, it can all be manipulated and accessed using the exactly same functions and APIs as the normal files in the system.
This means that all of the shell programs we've seen can be used not just to operate on files, but also *on processes* or *devices*!

So what does it look like to access files?
The relevant functions include:

- `int open(path, flags)` - Open a file, identified by its `path`, and return a *file descriptor* that can be used to access that file.
    The `flags` must be include of the following: `O_RDONLY`, `O_WRONLY`, or `O_RDWR` -- if you want to only read from the file, only write to the file, or have the ability to both read and write to the file.
	Another interesting `flag` value is `O_CREAT` which will *create* the file if it doesn't already exist.
	Whenever you see "flags", you should think of them as a set of bits, and each of the options as a single bit.
	Thus, when passing in flags, you can use bitwise operators to pass in multiple options, for example `open("penny_is_best.gif", O_RDWR | O_CREAT, 0777)` will open the file, creating it if it doesn't already exist, and enable reading and writing to the file.
- `read` &` write` - We've seen these before when we saw `pipe`s!
    They are the generic functions for getting data from, and pushing data to descriptors.
- `close` - Remember that we can `close` any descriptor to release it (the `free` for descriptors, if you will), including those that reference files.
- `int stat(path, struct stat *info)` - Get information about a file identified by `path` into the `info` structure.
    The structure is documented in the `man` page, but it includes, for example, the file size.
	`fstat` enables to you get stat info from an existing file descriptor.
- `creat(path, mode)` - Create a new file at the `path` with the `mode` (specified identically to `open`).
    Later, we'll learn about mode when we discuss security, for now, you can use a permissive mode of `0777`.
- `unlink(path)` - Try and remove a file!
    This is called, for example, by the `rm` program.


```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#define TWEET_LEN 280

int
main(void)
{
	struct stat finfo;
	char tweet[TWEET_LEN];
	int ret;

	/* can use paths that are relative to the current working directory */
	int fd = open("./05/prufrock.txt", O_RDONLY);
	assert(fd >= 0); /* should normally do better error checking */

	ret = fstat(fd, &finfo);
	assert(ret >= 0);
	printf("We want to say %ld things...but the world doesn't value artistry.\n", finfo.st_size);

	printf("Lets cut down to 280 characters and tweet like it is 1915...\n\n");
	fflush(stdout); /* see later */

	ret = read(fd, tweet, TWEET_LEN);
	assert(ret == TWEET_LEN); /* for this demo... */
	write(STDOUT_FILENO, tweet, TWEET_LEN);

	close(fd); /* we close file descriptors just the same */

	printf("\n");

	return 0;
}
```

> It is *really* important that quite a few interesting functions operate on *descriptors*, which might reference pipes, files, the terminal, or other resources.
> This means that processes can operate on descriptors and not care what resources back them.
> This enables shells to implement pipelines and the redirection of output to files, all without the processes knowing!
> It enables `fork`ed processes to inherit these resources from its parent.
> In this way, descriptors and the functions to operate on them are a *polymorphic* foundation (in the sense of object-oriented polymorphism) to accessing the system.

We see something a little strange with files as compared to `pipe`s.
Files have a finite size, and the data remains in the file after being read -- unlike with pipes where data is removed out of the pipe when read.
For files, subsequent reads and writes must progress through the contents of the file, but we must also to be able to go back and read previously read data.
Lets understand what happens when we `read` or `write` from a file -- we'll focus only on `read` to start, but both behave similarly.
Each file descriptor^[I'll try and call it a "file descriptor" when we know the descriptor is to a file.] tracks an *offset* into the file at which the next `read` will access data.
Lets go through an example of a file that contains the alphabet:

```
abcdefghijklmnopqrstuvwxyz
```

A freshly `open`ed file returns a file descriptor (`fd`) where `offset = 0`.

```
abcdefghijklmnopqrstuvwxyz
^
|
+-- offset
```

A `read(fd, buf, 10)` that returns `10` (saying that `10` bytes or characters `a` through `j`  was successfully read) advances `offset` by `10`.

```
abcdefghijklmnopqrstuvwxyz
          ^
          |
          +-- offset
```

Thus, an additional read will start reading from the file at `k`.
The offset, then, enables subsequent reads to iterate through a file's contents.
`write` uses the offset identically.

There are many cases in which we might want to modify the `offset`.
For example, databases want to jump around a file to find exactly the interesting records, playing audio and video requires us to skip around a file finding the appropriate frames.
Thus, we have the function:

- `off_t lseek(int fd, off_t update, int whence)` - This function lets you modify the `offset` of the file associated with `fd`, by `update`.
    The last argument can be set to `SEEK_SET` to set the `fd`'s `offset` to `update`, `SEEK_CUR` to *add* `update` to the current `offset` (thus making a relative update), or `SEEK_END` to set the `offset` to the files *end* plus `offset`.
	It returns the resulting `offset` of the `fd`.

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#define TWEET_LEN 280

int
main(void)
{
	char tweet[TWEET_LEN];
	int ret;

	/* can use paths that are relative to the current working directory */
	int fd = open("./05/prufrock.txt", O_RDONLY);
	assert(fd >= 0); /* should normally do better error checking */

	printf("What lies beyond the tweet? Tweet 2/N:\n\n");
	fflush(stdout); /* see later */

	ret = lseek(fd, 280, SEEK_SET);
	assert(ret == 280);

	ret = read(fd, tweet, TWEET_LEN);
	assert(ret == TWEET_LEN); /* for this demo... */
	write(STDOUT_FILENO, tweet, TWEET_LEN);

	close(fd); /* we close file descriptors just the same */

	printf("\n");

	return 0;
}
```

## Mapping Files

It is very common to access files with `read` and `write`.
However, they require many interactions with the file through subsequent calls to these functions.
Thus, UNIX provides a second means to access files.
UNIX also provides a way to directly "map" a file into a process, enabling to appear directly in memory, thus be accessible using normal memory accesses.
This is cool because your changes *in memory* update the file directly!

- `void *mmap(addr, len, prot, flags, fd, offset)` - Wow, that's a lot of arguments.
    Believe it or not, it was the call in Linux that had the *most* arguments for a long time!
	Lets go through these:

	- `fd` - this is the file descriptor we want to map into memory.
	- `addr` - this is the address in memory at which you want to map.
	    Normally, this is `NULL`, which means "please map it whereever you want".
		The *return value* is the address at which the file is mapped, or `NULL` if there was an error (see the `man` page and `errno` values).
	- `len` - How much of the file do we want to map in?
	    This is where `stat` is quite useful to find the entire file's size if you want to map it all in.
	- `offset` - If we aren't mapping the entire file, we can start mapping at this `offset`.
	- `prot` - This enables you to choose some properties for the mapping.
	    `prot` values include `PROT_READ`, and `PROT_WRITE` which ask the system to map the memory in readable, or writable (you can choose both with `PROT_READ | PROT_WRITE`).
		Note that the type of accesses here must not request accesses not previously requested in `open` -- for example, if we passed `O_RDONLY` in to `open`, `mmap` will return an error if we ask for `PROT_WRITE`.
	- `flags` - For now, lets focus on using only `MAP_PRIVATE`.
- `munmap` - Unmap a previously mapped file.

```c
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int
main(void)
{
	int ret;
	char *addr;

	/* can use paths that are relative to the current working directory */
	int fd = open("./05/prufrock.txt", O_RDONLY);
	assert(fd >= 0); /* should normally do better error checking */

	addr = mmap(NULL, 280 * 2, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(addr);

	/* The file is now memory we can access! */
	ret = strncmp(&addr[5], "Love", 4);
	assert(ret == 0);

	/* write it out to see it is the same as before */
	ret = write(STDOUT_FILENO, addr, 280 * 2);
	assert(ret == 280 * 2);

	munmap(addr, 280 * 2);

	return 0;
}
```

Note that you cannot `mmap` a `pipe` or terminal, only files.
This kinda makes sense: since a pipe holds *transient* data that is only there *until read out of the pipe* it doesn't make sense for it to be at a fixed address in memory -- it would require an infinite amount of memory as more and more data is written into the pipe!

**Memory allocation with `mmap`.**
Interesting, `mmap` can also be used to *allocate memory*.
Specifically, if we pass in `fd = 0`, `prot = PROT_READ | PROT_WRITE`, and `flags = MAP_ANONYMOUS | MAP_PRIVATE`, `mmap` will simply return newly allocated memory instead of a file!
Somewhere deep in `malloc`'s implementation, it calls `mmap` to allocate the memory it eventually returns to the application!

## Stream-based I/O

There is actually a *third* way to access files beyond "raw" `read`/`write` and `mmap` -- streams!
[Streams](https://www.gnu.org/software/libc/manual/html_node/I_002fO-on-Streams.html) are an abstraction *on top of the* "raw" descriptor accesses using `read` and `write`.

Streams are identified by the type `FILE *`, and all the APIs for operating on streams take `FILE`s instead of file descriptors.
In most ways they behave very similar to normal descriptor accesses -- you can `fopen` files to open a stream to a file, and `fread` and `fwrite` to read data from, and write data to the file.
As with a process' descriptors, we have default streams:

- `stdout`, `stdin`, `stderr` which correspond to `STDOUT_FILENO`, `STDIN_FILENO`, and `STDERR_FILENO`, respectively.

Some of the stream functions:

- `FILE *fopen(path, mode)` - similar to `open`, but returning a stream.
    The access rights we're asking for (reading or writing to the file), are specified in a more intuitive `mode` string.
	"r" to read, "r+" for read/write, etc...
- `fclose` - similar to `close`, but for `FILE *`s.
- `fread`/`fwrite` - Stream equivalents of `read`/`write`.
    Additionally, streams provide the `feof` function that tells us if we are at the end of a file.
- `printf`/`fprintf` - Wait, what?
    Yes, `printf` is actually used to send our string to the `stdout` stream.
	`fprintf` enables us to write the string we're creating out to a *specific* stream, not just the `stdout`!
- `scanf`/`fscanf` - Similar, `scanf` inputs from the `stdin`, and `fscanf` inputs from a specific stream.

```c
#include <stdio.h>
#include <assert.h>

int
main(void)
{
	FILE *f;
	char tweet_thread[(280 * 2) + 1];
	int ret;

	f = fopen("./05/prufrock.txt", "r");
	assert(f);

	ret = fread(tweet_thread, 1, 280, f);
	assert(ret == 280);
	ret = fread(&tweet_thread[280], 1, 280, f);
	assert(ret == 280);
	tweet_thread[280 * 2] = '\0';

	fprintf(stdout, "%s\n", tweet_thread);

	fclose(f);

	return 0;
}
```

**Streams as buffered I/O.**
So it doesn't seem like we're getting much of use out of these streams compared to raw descriptors.
Streams are an optimization over `read`/`write` as they *buffer* data.
Buffering essentially means that your `fwrite`s actually write data into a "buffer" in memory, and only when either you write out a `\n` or when the buffer gets large enough does a `write` get called to send the data to the system.
Why do this?
By buffering data in memory rather than making a `write` for each `fwrite`, we're saving any overhead associated with each `write` (to be discussed in a couple of weeks).
However, this means that just because we called `printf`, doesn't mean it will output immediately!
Yikes.
Lets see this.

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	printf("hello world");
	_exit(EXIT_SUCCESS); /* this exits immediately without calling `atexit` functions */

	return 0;
}
```

`_exit` exits from the process *immediately*, and without invoking any `atexit` functions.
Flushing the `stdout` stream is done in one such function, thus we don't see the output!

```c
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	printf("hello world");
	fork();
	printf("\n"); /* remember, streams output on \n */

	return 0;
}
```

Well that's not good.
The *buffered data* is copied into each process, then it is output when a `\n` is encountered.

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int
main(void)
{
	printf("hello ");
	write(STDOUT_FILENO, "world ", 6);
	printf("\n"); /* remember, streams output on \n */

	return 0;
}
```

Both `printf` and the `write ` are to standard output, but `printf` is to a stream.
The *buffered* `printf` output is not written out immediately, thus ordering can get messed up.
Yikes.

It is imperative that streams give us some means to force the buffer to be output!
Thus, streams provide a means of *flushing* the stream's buffers, and sending them out to the system (e.g. using `write`).

- `fflush` - flush out the buffer associated with a specific stream.

Fixing the previous examples:

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	printf("hello ");
	fflush(stdout);
	write(STDOUT_FILENO, "world ", 6);
	printf("\n"); /* remember, streams output on \n */

	printf("hello world");
	fflush(stdout);
	_exit(EXIT_SUCCESS); /* this exits immediately without calling `atexit` functions */

	return 0;
}
```

```c
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	printf("hello world");
	fflush(stdout);
	fork();
	printf("\n"); /* remember, streams output on \n */

	return 0;
}
```

A few other functions for streams that can be useful:

- `fileno` - Get the file descriptor number associated with a stream.
    This can be useful if you need to use an API that takes a descriptor, rather than a stream `* FILE`.
- `getline` - Read out a line (delimited by a `\n`) from a stream.
    Since reading input, line at a time, is a pretty common thing, this is a useful function.

```c
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
	FILE *f = fopen("./05/prufrock.txt", "r");
	size_t s = 0;
	char *line = NULL;
	int i;

	assert(f);
	printf("The new genre: abridged, tweet-worthy poetry...\n\n");
	for (i = 0; getline(&line, &s, f) != -1; i++) {
		if (i % 15 == 0) {
			fwrite(line, 1, strlen(line), stdout);
			/* same as printf("%s", line); */
		}
	}
	if (!feof(f)) {
		perror("getline");
		exit(EXIT_FAILURE);
	}
	free(line);
	fclose(f);

	return 0;
}
```

## Accessing Directories

Files aren't the only thing we care to access in the system -- what about directories!
We want to be able to create directories, delete them, and be able to read their contents.
These core operations are what underlies the implementation of the `ls` program, or, similarly, what happens when you double-click on a directory in a graphical file explorer!

- `opendir`, `closedir` -
    Open (and close) a directory, and return a *directory stream*, a `DIR *`, to it, which is backed by a descriptor.
	Yes, another descriptor!
- `readdir` -
    This takes a `DIR *` to a directory, and returns a structure that includes the "current" file or directory in the directory.
	This includes the name of the returned object in `d_name`, and type (in the `d_type` field) which is one of:

    - `DT_BLK` -   This is a block device.
    - `DT_CHR` -   This is a character device.
    - **`DT_DIR` -   This is a directory.**
    - `DT_FIFO` -  This is a named pipe (FIFO).
    - `DT_LNK` -   This is a symbolic link.
    - **`DT_REG` -   This is a regular file.**
    - `DT_SOCK` -  This is a UNIX domain socket.
    - `DT_UNKNOWN` - The file type could not be determined.

	The two main ones we care about are directories and files.
	Subsequent calls to `readdir` will return the "next" file or directory.
- `scandir` & `glob` -
    Two functions that enable you to get directory contents based on some simple search patterns and logic.
	`scandir` lets you pass in a directory stream, and a couple of functions that are used to sort directory contents output, and filter it (enable you to say that you don't want to look at some directory contents).
	`glob`, on the other hand, lets you search for "wildcards" designated by `*` to select all files/direcotries that match the string.

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long file_size(char *dir, char *file);

int
main(void)
{
	DIR *d = opendir("./05/");
	struct dirent *entry;

	assert(d);

	errno = 0;
	while ((entry = readdir(d)) != NULL) {
		char *type;

		if (entry->d_type == DT_DIR)      type = "Directory";
		else if (entry->d_type == DT_REG) type = "File";
		else                              type = "Unknown";

		if (entry->d_type == DT_DIR || entry->d_type != DT_REG) {
			printf("%10s: %s\n", type, entry->d_name);
		} else { /* entry->d_type == DT_REG */
			printf("%10s: %s (size: %ld)\n", type, entry->d_name, file_size("05", entry->d_name));
		}
	}
	if (errno != 0) {
		perror("Reading directory");
		exit(EXIT_FAILURE);
	}
	closedir(d);

	return 0;
}

long
file_size(char *dir, char *file)
{
	struct stat finfo;
	char buf[512];
	int ret;

	memset(buf, 0, 512); /* zero out the buffer to add '\0's */
	snprintf(buf, 512, "./%s/%s", dir, file);

	ret = stat(buf, &finfo);
	assert(ret == 0);

	return finfo.st_size;
}
```

To make *changes* in the first system hierarchy, we need an additional set of functions.

- `mkdir(path, mode)` -
    This function is used in, for example, the `mkdir` program.
	Don't confuse the program you use in the shell, with the function you can call from C.
- `rmdir(path)` -
    Deletes a directory that is empty.
	This means that you have to `unlink` and `rmdir` the directory's contents first.
- `rename(from, to)` -
    Change the name of file or directory from `from` to `to`.
	You can imagine this is used by the `mv` command line program.

```c
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int
main(void)
{
	int ret;
	int fd;

	ret = mkdir("05/newdir", 0777);
	assert(ret == 0);
	fd = open("05/newdir/newfile", O_RDWR | O_CREAT);
	assert(fd >= 0);
	ret = write(fd, "new contents", 13);
	assert(ret == 13);
	ret = rename("05/newdir", "05/newerdir");
	assert(ret == 0);
	ret = unlink("05/newerdir/newfile");
	assert(ret == 0);
	ret = rmdir("05/newerdir");
	assert(ret == 0);

	printf("If there were no errors, we\n\t"
		"1. created a directory, \n\t"
		"2. a file in the directory, \n\t"
		"3. change the directory name,\n\t"
		"4. removed the file, and\n\t"
		"5. removed the directory\n");

	return 0;
}
```
