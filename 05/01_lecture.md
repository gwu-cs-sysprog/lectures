
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

- `open` - Open a file, identified by its *path*, and return a *file descriptor* that can be used to access that file.
- `read` &` write` - We've seen these before when we saw `pipe`s!
    They are the generic functions for getting data from, and pushing data to descriptors.
- `close` - Remember that we can `close` any descriptor to release it (the `free` for descriptors, if you will).
- `stat` - Get information about a file, for example its size.

```c
/* open, stat, read, & close */
```

We see something a little strange with files as compared to `pipe`s.


- `lseek` -

### Mapping Files

- mmap/munmap

### Stream-based I/O

[Streams](https://www.gnu.org/software/libc/manual/html_node/I_002fO-on-Streams.html) are an abstraction on top of the "raw" file access using `read` and `write`.


- stream functions (fopen, fclose, fread, fwrite, printf, fprintf, fflush, fscanf, feof, fileno, getline...)

    - stdout, stdin, stderr

### Accessing Directories

- directories

	- opendir, closedir
	- readdir, scandir
	- glob
