
## Process Exercises

### Exercise 1: fibonacci with `fork`

Implement `forkonacci`!
This will solve the `n`th [fibonacci number](https://en.wikipedia.org/wiki/Fibonacci_number), using `fork` for the recursion, and a combination of `exit` to "return" and `wait` to retrieve the returned value, instead of function calls.
The normal recursive implementation:

``` c
unsigned int
fibonacci(unsigned int n)
{
    if (n == 0 || n == 1) return n;

	return fibonacci(n - 1) + fibonacci(n - 2);
}
```

This will work decently for `n <= 12`, but not for `n > 12`.
Why^[Hint: make sure to read the `man` pages for `exit` (`man 1 exit`).]?

### Exercise 2: Shell Support for Environment Variables

How do you think that the shell-based support for environment variables (`export BESTPUP=penny`) is implemented?
Specifically, if we do the following...

```
$ export BESTPUP=penny
$ ./03/envtest.bin BESTPUP  // this just prints the environment variable
Environment variable BESTPUP -> penny
```

...which process is using which APIs?
Put another way, how is `export` implemented?

### Exercise 3: Process Detective

The `/proc` filesystem is a treasure trove of information.
You can dive into the process with `pid` `N`'s information in `/proc/N/`.
You'll only have access to the informationx for your processes.
So how do you find the `pid` of your processes?
You can find the `pid` of all of the processes that belong to you using `ps aux | grep gparmer  | awk '{print $2}'`^[`ps` prints out process information, `grep` filters the output to only lines including the argument (`gparmer` here), and `awk` enables us to print out specific columns (column `2` here). You should start getting familiar with your command-line utilities!], but replacing your username for `gparmer`.

Choose one of those, and go into its directory in `/proc`.
It it doesn't work, you might have chosen a process that has terminated since you find its ID.
Try again.

What do you think the following files contain:

- `cmdline`
- `environ`
- `status`

Some of the files are *binary*, which means you might need to use `xxd` (a hexdecimal binary view program) to look at them.
In `xxd`, pay attention to the right column here.
