
## Exercises

Write a `tree` clone, with disk usage information.
Though it isn't installed, by default, tree outputs the filesystem at a specific directory:

```
$ tree .
.
├── 01_lecture.md
├── 02_exercises.md
├── prufrock.txt
└── test_directory
    └── crickets.txt
```

### Task 1: Markdown Tree

We'll simplify the output to print out markdown lists with either `D` or `F` for directories or files.
If we run the program in the `05` directory:

```
$ ./t
- 01_lecture.md (20759)
- 02_exercises.md (1683)
- prufrock.txt (6044)
- test_directory
    - crickets.txt (26)
```

Some starter code that focuses on printing out info for a single directory.

**Task: Change this code to print out the *hierarchy* of files and directories, rather than just the contents of a single directory.**

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* helper functions */
void indent(int n);
int ignoredir(char *dir);
long file_size(char *dir, char *file);

/*
 * Return the size of the directory (sum of all files inside).
 * The easiest implementation here is recursive where this is called
 * for each directory.
 */
size_t
print_dir(char *dir, int depth)
{
	DIR *d = opendir(dir);
	struct dirent *entry;

	assert(d);
	errno = 0;

	/* Go through each dir/file in this directory! */
	while ((entry = readdir(d)) != NULL) {
		/* we'll ignore . and .. */
		if (ignoredir(entry->d_name)) continue;

		/* print out the proper indentation */
		indent(depth);
		if (entry->d_type == DT_DIR) {
			printf("- D %s\n", entry->d_name);
		} else if (entry->d_type == DT_REG) {
			printf("- F %s (%ld)\n", entry->d_name, file_size(dir, entry->d_name));
		}
		/* we'll ignore everything that isn't a file or dir */
	}
	if (errno != 0) {
		perror("Reading directory");
		exit(EXIT_FAILURE);
	}
	closedir(d);

	return 0;
}

int
main(void)
{
	print_dir("./", 0);

	return 0;
}

/* Indent `n` levels */
void
indent(int n)
{
	for (int i = 0; i < n; i++) printf("    ");
}

/* Should we ignore this directory? */
int
ignoredir(char *dir)
{
	return strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0;
}

long
file_size(char *dir, char *file)
{
	struct stat finfo;
	char buf[512];
	int ret;

	memset(buf, 0, 512); /* zero out the buffer to add '\0's */
	snprintf(buf, 512, "%s/%s", dir, file);

	ret = stat(buf, &finfo);
	assert(ret == 0);

	return finfo.st_size;
}
```

### Task 2: File and Directory Sizes

**Task: Add output for the size of files, and the size of each directory -- the sum of the size of all contained directories and files.**

Again, if we run the program in the `05` directory:
```
$ ./t
- 01_lecture.md (20759)
- 02_exercises.md (1683)
- prufrock.txt (6044)
- test_directory
    - crickets.txt (26)
	(26)
(28512)
```

The `test_directory` size is the `(26)`, while the `./` directory has size `(28512)`.
Your specific sizes might differ.
