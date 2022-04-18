
# UNIX Security: Users, Groups, and Filesystem Permissions

> **This material adapted from Prof. Aviv's [material](https://classes.adamaviv.com/ic221/s18/units/01/unit.html#org26c0ea7).**

We've seen how the filesystem is used to store and name files and channels, and we've seen the APIs to access directories and files.
If everyone had access to every else's files, then there would be no way to secure your information.
Thus, the kernel provides means to control different user's *access* to different files and directories.
This means that I cannot access your files, and vice-versa!

To understand the core concept of *access control* in UNIX, we have to understand

1. how UNIX thinks about, implements, and organizes users and groups of users,
2. how files and directories provide various access permissions to different users and how to alter those permissions, and
3. how services, even when executed by a user, can have heightened access to system resources.

## File and Directory Permissions

The filesystem stores files and directories are somehow associated with each of us, separately.
Lets check out the properties of a file that are returned by running `ls -l`:

```
.- Directory?
|    .-------Permissions                   .- Directory Name
| ___|___     .----- Owner                 |
v/       \    V     ,---- Group            V
drwxr-x--x 4 aviv scs 4096 Dec 17 15:14 im_a_directory
-rw------- 1 aviv scs 400  Dec 19  2013 .ssh/id_rsa.pub
                       ^   \__________/    ^
File Size -------------'       |           '- File Name
  in bytes                     |
                               |
   Last Modified --------------'
```

An example from the class' server where we create a file, a directory, and see the results:

```
$ echo hi > file.txt
$ mkdir hello
$ ls -l
-rw-r--r-- 1 gparmer dialout    3 Mar 18 12:19 file.txt
drwxr-xr-x 2 gparmer dialout 4096 Mar 18 12:20 hello
```

We see that each file and directory has a owner and group associated with it.

**Question:**

- What do you think this means?
    Why do files have owners and groups?

### File Ownership and Groups

The *owner* of a file is the *user* that is directly responsible for the file and has special status with respect to the file permissions.
Users can also be collected together into a set called a *group*, a collection of users who posses the same permissions.
A file is associated with a group.
A file's or directory's owners and groups have different permissions to access the file.
Before we dive into that, lets understand how to understand users and groups.

You all are already aware of your username.
You can get your username with the `whoami` command:

```
$ whoami
gparmer
```

To have UNIX tell you your username and connection information on this machine, use the command, `who`:

```
$ who
gparmer  pts/1        2022-03-18 12:12 (128.164...)
```

The first part of the output is the username.
The rest of the information in the output refers to the terminal you're using (it is a "pseudo-terminal device", something that pretends it is a typewriter from the 60s), the time the terminal was created, and from which host you are connected.

You can determine which groups you are in using the groups command.

```
$ groups
dialout CS_Admins
```

On the class' server, I'm in the `dialout` and `CS_Admins` groups.
On my work computer, I'm in a few more groups:

```
$ groups
ycombinator adm cdrom sudo dip plugdev lpadmin sambashare
```

The good news is that I can use my `cdrom`.
Fantastic.
It will pair well with my pseudo-typewriter.
Cool.

### File Permissions

The *permissions* associated with each file/directory specify who should be able to have the following access:

- `r` - the ability to *read* the file, or *read the contents* of a directory.
- `w` - the ability to *write* to the file, or update the contents of a directory.
- `x` - the ability to *execute* a file (i.e. a program), or `chdir` into a directory.

We think about different users as having different sets of these permissions to access files and directories.
When we see the permissions string for a file, we see three sets of these `rwx` permissions:

```
-rw-r--r-- 1 gparmer dialout    3 Mar 18 12:19 file.txt
```

Here we have three sets of permissions `rw-` (read and write), `r--` (read only), and `r--` (read only).
These three sets of permissions each specify the access permissions for the *owner*, the *group*, and everyone else's *global* permissions.

```
 .-- Directory Bit
|
|       ,--- Global Permission (i.e. "every else")
v      / \
-rwxr-xr-x
 \_/\_/
  |  `--Group Permission
  |
   `-- Owner Permission
```

Given this, now we can understand the permissions for each file in:

```
ls -l 11/perm_example/
total 0
-rw-r--r-- 1 gparmer gparmer 0 Mar 25 18:43 bar
-rw-rw-r-- 1 gparmer gparmer 0 Mar 25 18:43 baz
-rwx------ 1 gparmer gparmer 0 Mar 25 18:42 foo
```

*Question:*

- *Who* has *what access permissions* to which files?

### Numerical Representation of Permissions

Each permission has a *numerical representation*.
We represent it in *octal*, which is a fancy way to say that we think of each digit of our numerical representation as being between `0-7` -- it is "base 8" (in contrast to the normal digits numbers we use every day that are "base 10").
Thus, each of the three sets of permissions is `3` bits.

Each of the three permissions, `r`, `w`, and `x` correspond to the most-significant to the least significant bits in each octal digit.
For example:

```
rwx -> 1 1 1 -> 7
r-x -> 1 0 1 -> 5
--x -> 0 0 1 -> 1
rw- -> 1 1 0 -> 6
```

On the left we see a permission, its bit representation, and the associated octal digit.
The shorthand, you can think of this is this:

- `r = 4` as it is represented with the most significant bit
- `w = 2` as the second bit
- `x = 1` as it is the least significant bit

Thus the octal digit is simply the addition of each of these numeric values for each access right.
We can combine these digits to encapsulate permissions for the owner, group, and everyone else:

```
-rwxrwxrwx -> 111 111 111 -> 7 7 7
-rwxrw-rw- -> 111 110 110 -> 7 6 6
-rwxr-xr-x -> 111 101 101 -> 7 5 5
```

So now we know how to interpret, and understand these permission values, and how to represent them as digits.

### Updating File/Directory Permissions

To change a file permission, you use the `chmod` command, or the `chmod` system call.
In each, we specify the file to update, and the new permissions in octal.
Lets play around with permission and see their impact:

```
$ gcc helloworld.c -o helloworld.bin
$ ls -l helloworld.bin
-rwxrwxr-x 1 gparmer gparmer 41488 Mar 22 08:52 helloworld.bin
$ ./helloworld.bin
hello world!
```

I can execute the program, and I could read the program, for example using `nm`.
I could even overwrite the file.

> Note: on the server, the default permissions are more restrictive --
> `-rwxr-xr-x 1 gparmer dialout 16464 Mar 25 19:16 helloworld.bin`.
> We don't want anyone other than us writing to the file on a shared server!

What if I removed all permissions for everyone but the owner, and even removed my own ability to execute?

```
$ chmod 600 helloworld.bin
$ ls -l helloworld.bin
-rw------- 1 gparmer gparmer 41488 Mar 22 08:52 helloworld.bin
$ ./helloworld.bin
-bash: ./helloworld.bin: Permission denied
```

We removed our own ability to execute the file, so we no longer can!
Note that permission `6` is `4 + 2` which is `r` + `w`.
If I wanted on the system to be able to execute the program:

```
$ chmod 711 helloworld.bin
$ ls -l helloworld.bin
-rwx--x--x 1 gparmer gparmer 41488 Mar 22 08:52 helloworld.bin
$ ./helloworld.bin
hello world!
```

Recall, `7 = 4 + 2 + 1` which is `r` + `w` + `x`.
We can see that we've given everyone on the system execute permissions, so the file is again about to execute!
Any other user on the system would also be able to execute the program.

**Questions:**

- How do you think permissions change access to directories?
    Try out setting the permissions of a directory (recall `mkdir`), and seeing what accesses cannot be made if permissions are removed.
- You and a classmate should sit down together, and use `chmod` to concretely understand when you can access each other's files.
    Remember, you can find out where your files are with `pwd`, what groups you're in with `groups`.
	Which files can another user can access at that path with sufficient permissions?
	You can use the `/tmp/` directory to store files that you can both mutually access, permissions allowing.

### Changing File/Directory Owner and Group

We've seen that each file and directory is associated with a user, and with a group.
Which means that, of course, we can change those settings!
Two commands (and their corresponding system call: `chown`):

- `chown <user> <file/directory>` - change owner of the file/directory to the user
- `chgrp <group> <file/directory>` - change group of the file to the group

UNIX limits who is able to execute these operations.

- Only a *superuser* called `root` is allowed to `chown` a file or directory.
    `root` is special in that they are able to change the permissions of any file/directory, and most of the most sensitive files on the system are owned by root.
	For example, the devices (think, hard-drives holding the filesystem, monitors, and network devices) are only accessible by `root` (check out `/dev/*`).
- Only the *owner* of a file (and `root`) is allowed to change the group of a file, and only to a group that user is in (recall `groups`).

We can see this:

```
$ chown root helloworld.bin
chown: changing ownership of 'helloworld.bin': Operation not permitted
```

**Question:**

- Why do you think we aren't allowed to change the owner of a file, nor change the group to one in which we don't belong?

## Security and Programming with Users and Groups

We've seen that a lot of UNIX security centers around users, groups, and the corresponding permissions associated with files and directories.
When a process is `fork`ed, it *inherits* the user and group of the parent process.
Thus, when you're typing at a shell, the *shell process* is what really is executing as your user id, and when you create programs, they inherit your user id.
These are the core of the security primitives, so lets dive into a little more detail.

#### APIs for Accessing a Program's Privilege Settings

If every process has a user and group, we must be able to access them, right?
There are two basic system calls for retrieving user and group information within a program.

- `uid_t getuid(void)` - Returns the real user id of the calling process.
- `gid_t getgid(void)` - Returns the real group id of the calling process.

Let's look at an example program.

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char * argv[])
{
	uid_t uid;
	gid_t gid;

	uid = getuid();
	gid = getgid();

	printf("uid=%d gid=%d\n", uid, gid);

	return 0;
}
```

Every user has different identifiers as does each group.
If another user where to run the same program, they'd get a different value.

**Question:**

- In groups, one of you compile this program, and the other run it!
    Use `chmod` to make sure that they can!
	What do you observe?
- If the kernel thinks of users as numeric identifiers, how does `whoami` print out a *human-readable string*?
    Use `strace whoami` to come up with a strong guess.
- How do we get human-readable groups with `groups` (investigate with `strace groups`)?

Interestingly, whomever *executes* the program will have their uid/gid printed out; it won't use the uid/gid of the owner of the program.
Remember, that we inherit the uid/gid on fork, thus when the shell runs the program, it uses our shell's (thus our) uid/gid.

### User and Group Strings

If the UNIX programmatic APIs provide access to user and group *identifiers* (i.e. integers), how is it that we can see strings corresponding to these numeric values?
The mapping of the identifiers to strings are defined in two places.
The first is a file called `/etc/passwd` which manages all the users of the system.
Here is the `/etc/passwd` entry on my local system (the server uses another means to store user info):

```
$ cat /etc/passwd | grep gparmer
gparmer:x:1000:1000:Gabe Parmer,,,:/home/ycombinator:/bin/bash
   |    |   |     |      |                 |             |
   V    |   V     |      V                 V             V
 user   | user id |  human-readable id  home directory  shell to use upon login
        V         V
    password    group id
```

We can lookup our username and associate it with a numerical user id, and a group id for that user.
These numbers are what the system uses to track users, but UNIX nicely converts these numbers into names for our convenience.
The file additionally holds the path to the user's home directory, and their shell.
These are used upon the user logging in: the login logic will switch to the user's home directory, and execute their shell.
The `password` entry is deprecated.
The translation between userid and username is in the password file.
The translation between groupid and group name is in the group file, `/etc/group`.
Here is the entry for the administrators on the class' server:

```
$ grep CS_Admins /etc/group
CS_Admins:x:1004:aaviv,timwood,gparmer,...
```

There you can see that the users `aaviv`, `timwood`, and `gparmer` are all in the group of administrators.

All of this information, including identifiers, and human-readable strings can be retrieved with `id`.

```
$ id
uid=1000(gparmer) gid=1000(gparmer) groups=1000(gparmer),4(adm),24(cdrom),27(sudo),30(dip),46(plugdev),120(lpadmin),131(lxd),132(sambashare)
```

**Question:**
Take a moment to explore these files and the commands.
See what groups you are in.

## Application-Specific Privileges using set-user-id/set-group-id

Systems that support multiple users are quite complex.
They have services used by many users, but that must be protected from the potentially accidental or malicious actions of users.
These programs require more privileges than a user to do their job.
A few examples:

- *Logging* is a very important functionality in systems.
    It enables services to log actions that are taken, for example, users logging in to `ssh` and clients connecting to webservers.
    The logs are stored in files (often in `/var/log/`), and users must *not* be able to directly modify (and potentially corrupt) the logs.
	But they must be able to append to the logs.
	How can we support this contradiction when a logging process will run with our user id when we execute it!?
- *Coordinating actions* such as running commands regularly, or at specific points in time.
    The programs `at` and `cron` enable exactly this!
	They require adding entries into system files (e.g. `crontab`) to record the actions, but one user must not corrupt the entires of another user.
	Again, we want to both be able to add entries to the files, but cannot modify the file!

We already know one way to solve this problem.
We can write a service, and have clients *request* (e.g. via IPC and domain sockets) it perform actions on our behalf.
So long as a service runs as a different user, its files will be in accessible from clients.

However, there is another way to solve the above contradictions.
We really want a program to run with a *different* user's permissions -- those that go beyond our own.
We want the *logger* to run with the permissions of the logging user, and we want `cron` to run with the permissions of the `cron` service.
This is possible with the set-uid-bits (and set-gid-bits).

The `set-*-bits` can be seen in the `man chmod` manual page.
We're used to permissions being three digits (for the owner, the group, and everyone else), but it can be specified as *four* digits.
The most significant digit can be set on executable binaries, and will result in the binary executing with the owner's uid/gid.

That is, we previously assumed a permission string contained 3 octal digits, but really there are 4 octal digits.
The missing octal digit is that for the set-bits.
There are three possible set-bit settings and they are combined in the same way as other permissions:

- `4` - *set-user-id*: sets the program's effective user id to the owner of the program
- `2` - *set-group-id*: sets the program's effective group id to the group of the program
- `1` - *the sticky bit*: which, when set on a directory, prevents others from deleting files from that directory.
    We won't cover the stick bit.

These bits are used in much the same way as the other permission modes.
For example, we can change the permission of our get_uidgid program from before like so:

```
chmod 6751 get_uidgid
```

And we can interpet the octals like so:

```
set      group
bits user |  other
  |   |   |   |
  V   V   V   V
 110 111 101 001
  6   7   5   1
```

When we look at the `ls -l` output of the program, the permission string reflects these settings with an "s" in the execute part of the string for user and group.

```
$ ls -l get_uidgid
-rwsr-s--x 1 aviv scs 8778 Mar 30 16:45 get_uidgid
```

#### Real vs. Effective Capabilities

With the set-bits, when the program runs, the capabilities of the program are effectively that of the owner and group of the *program*.
However, the real user id and real group id remain that of the user who ran the program.
This brings up the concept of *effective* vs. *real* identifiers:

- *real user id* (or *group id*): the identifier of the actual user who executed a program.
- *effective user id* (or *group id*): the idenifier for the capabilities or permissions settings of an executing program.

The system calls `getuid` and `getgid` return the *real* user and group identifiers, but we can also retrieve the *effective* user and group identifiers with

- `uid_t geteuid(void)` - return the effective user identifier for the calling process.
- `gid_t getegid(void)` - return the effective group identifer for the calling process.

We now have enough to test *set-x-bit* programs using a the following program that prints both the real and effective user/group identities.

``` c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	uid_t uid,euid;
	gid_t gid,egid;

	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	return 0;
}
```

As the owner of the file, after compilation, the permissions can be set to add set-user-id:

```
$ gcc 11/get_uidgid.c -o get_uidgid
$ chmod 6755 get_uidgid
$ ls -l get_uidgid
-rwsr-sr-x   1 gparmer        gparmer     16880 2022-03-29 08:30 get_uidgid
```

Notice the `s` values to denote the `set-*-bits`.

Lets test this set uid stuff!

```
$ cp get_uidgid /tmp/
$ chmod 4755 /tmp/get_uidgid
```

Now you should all have access to the file, with it being set as set-uid.

**Question:**

- Compile and run the `get_uidgid` program.
	What would you expect to happen?
    What do you observe?
	Recall that you can use `id` to print out both of your ids and groups.
- Run the `get_uidgid` program I placed in `/tmp/`.
    What is the expected output?
	What do you observe?
- What happens if we enable a program to create files while using the `set-*-id` bits?
    Lets use the following program that will create a file named after each command line arguments.

	``` c
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <fcntl.h>

	int
	main(int argc, char * argv[])
	{
		int i, fd;

		for(i = 0; i < argc; i++){
			/* create an empty file */
			if((fd = open(argv[i],O_CREAT,0666) > 0) > 0){
				close(fd);
			} else {
				perror("open");
			}
		}

		return 0;
	}
	```

	Run the following program in your home directory (can also be found in `create_files.c`), passing as arguments the names of files you want to create: `./create_files blah` will create a file `blah`.
	Check out the owner of the files (`ls -l blah`).
	I've also set up the file as **setuid** on the server in `/tmp/create_files`.
	Try running the following

	```
	$ /tmp/create_files /tmp/blah
	$ ls -l /tmp/blah
	...
	$ /tmp/create_files ~/blah
	...
	```

    What do you think the output for the first `...` will be -- who is the owner of the file?
	The second?
	What are the outputs, and why?

#### Programmatically Downgrading/Upgrading Capabilities

The set-bits automatically start a program with the effective user or group id set; however, there are times when we might want to *downgrade privilege* or change permission dynamically. There are two system calls to change user/group settings of an executing process:

- `setuid(uid_t uid)` - change the effective user id of a process to uid
- `setgid(gid_t gid)` - change the effective group id of a proces to gid

The requirements of `setuid()` (for all users other than root) is that the effective user id can be changed to the *real user id of the program* or to an *effective user id as described in the set-bits*.
The root user, however, can *downgrade to any user id* and upgrade back to the root user.
For `setgid()` the user can chance the group id to any group the user belongs to or as allowed by the set-group-id bit.

Now we can look at a program that downgrades and upgrades a program dynamically:

``` c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
	uid_t uid, euid;
	gid_t gid, egid;
	uid_t saved_euid;

	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	saved_euid=euid;
	setuid(uid);
	printf("---- setuid(%d) ----\n",uid);

	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	setuid(saved_euid);
	printf("---- setuid(%d) ----\n",saved_euid);
	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	return 0;
}
```

**Question:**

- What do think will happen if you have a peer run it on the server after you set the `set-uid` bit?

## Applications of Identify and Permission Management

### Logging Events: Selectively Increasing Access Rights

We previously covered the general desire to have a log in the system of the events for different activities.
For example, logging when each user logs in, when clients make web requests of an http server, etc...

We might support two high-level operations:

1. adding a logging event, and
2. reading the log.

If we don't care to censor the log, we can support the second very easily by, for example, making the log file's group the `log` group, and giving read (`4`) permissions to the log for anyone in that group.
Now any user in the `log `group can read the log.

However, to support adding logging events to the log, we might use the set-uid-bit to ensure that a `log` program runs with the effictive user-id of the logger user.
Thus, it will be able to append to the log in a straightforward manner.

### Logging In: Carefully Decreasing Access Rights

One example of how you might want to use the identity management APIs is when logging in.
The process of logging in requires that the `login` program access sensitive files that include (encrypted) user passwords (`/etc/shadow`).
Understandably, users don't generally have access to these files.
Thus, the act of logging in requires programs running as `root` to read in username and password.
Only if they match the proper values for a user, will the login logic then change uid/gid to the corresponding user and group id, and execute a shell for the user (as the user).
See the figure for more details.

![The login procedure on a system. On a system that uses a nameserver like `systemd`, it will ensure that terminals are available (either attached to the keyboard/monitor, or remote) by executing the [`getty` (link)](https://github.com/mirror/busybox/blob/master/loginutils/getty.c), or "get tty" program (or `ssh` for remote, or `gdm` for graphical) which makes available a terminal for a user to interact with. `getty` executes the [`login` (link)](https://github.com/mirror/busybox/blob/master/loginutils/login.c) program that reads the username and password from the user. If it is unable to find a match, it exits, letting `systemd` start again. If the username and password match what is in the `/etc/passwd` and `/etc/shadow` (password) files, it will change the user and group ids to those for the user in the `/etc/passwd` file, change to the home directory specified in that same file, and `exec` the corresponding shell.](figures/login.svg)

### `sudo` and `su`: Increasing Access Rights

There are many times in systems that we want to *increase* the privileges of a human.
For example, for security, it is often *not possible* to login as *root*.
But it is certainly true that sometimes humans need to be able to run commands as root.
For example, updating some software, installing new software, creating new users, and many other operations, all require *root*.

To control who is able to execute commands as *root*, the `sudo` and `su` programs enable restricted access to root privileges.
Specifically, `sudo` and `su` will execute a command as a *specified user* or *switch to a specified user*.
By default, these commands execute as the root user, and you need to know the root password or have sudo access to use them.

We can see this as the case if I were to run the `get_euidegid` program using `sudo`.
First notice that it is no longer set-group or set-user:

```
$ sudo ./get_euidegid
[sudo] password for gparmer:
 uid=0  gid=0
euid=0 egid=0
```

After sudo authenticated me, the program's effective and real user identification becomes 0 which is the uid/gid for the root user.

#### sudoers

Who has permission to run `sudo` commands?
This is important because on many modern unix systems, like ubuntu, there is no default root password.

**Question:**

- Why don't we just have a `root` account with a password instead of having `sudo` and `su`?

Instead certain users are deemed to be sudoers or privileged users.
These are set in a special configuraiton file called the `/etc/sudoers`.

```
aviv@saddleback: lec-23-demo $ cat /etc/sudoers
cat: /etc/sudoers: Permission denied
aviv@saddleback: lec-23-demo $ sudo cat /etc/sudoers
#
# This file MUST be edited with the 'visudo' command as root.
#
# Please consider adding local content in /etc/sudoers.d/ instead of
# directly modifying this file.
#
# See the man page for details on how to write a sudoers file.
#
Defaults	env_reset
Defaults	mail_badpass
Defaults	secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# Host alias specification

# User alias specification

# Cmnd alias specification

# User privilege specification
root	ALL=(ALL:ALL) ALL

# Members of the admin group may gain root privileges
%admin ALL=(ALL) ALL

# Allow members of group sudo to execute any command
%sudo	ALL=(ALL:ALL) ALL

# See sudoers(5) for more information on "#include" directives:

#includedir /etc/sudoers.d
```

Notice that only root has access to read this file, and since I am a `sudoer` on the system I can get access to it.
If you look carefully, you can perform a basic parse of the settings.
The root user has full `sudo` permissions, and other sudoer's are determine based on group membership.
Users in the `sudo` or `admin` group may run commands as `root`, and I am a member of the `sudo` group:

```
$ id
uid=1000(gparmer) gid=1000(gparmer) groups=1000(gparmer),4(adm),24(cdrom),27(sudo),30(dip),46(plugdev),120(lpadmin),131(lxd),132(sambashare)
```

**Question:**

- How do you think that `sudo` is implemented?
    What mechanisms from this week's lecture is it using to provide the requested functionality?

####  Assignment Submission

What if we implemented an assignment submission program that copied a student's directory into the instructor's home directory?
For example, if you wanted to submit the directory of code at `my_hw/` for homework `HW1`,

```
$ ./3410_submit HW1 my_hw
```

This program, run by you, has your user and group permissions, but it is able to take your submission and copy/save those submissions to my home directory, with my permissions at a location where you do not have access to write.
How is that possible?

Naively, we'd have a problem:

- The submission program is written and provided by the instructor.
- The students would be executing the program, thus it would execute with their identity.
- We don't want the submitted data to be visible to *any* students.
- *However*, if the students run the submission program, it will run as their uid/gid, and 1. it won't have access to the instructor's home directory, and 2. any files it creates (during the copy) will belong to the user.

Yikes.

**Question:**

- How could we go about implementing a homework submission program?
