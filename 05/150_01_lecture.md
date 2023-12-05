

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

int main(void)
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

long file_size(char *dir, char *file)
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

int main(void)
{
	int ret;
	int fd;

	ret = mkdir("05/newdir", 0700);
	assert(ret == 0);
	fd = open("05/newdir/newfile", O_RDWR | O_CREAT, 0700);
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
