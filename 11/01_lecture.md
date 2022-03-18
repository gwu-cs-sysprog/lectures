
# UNIX Security: Users, Groups, and Filesystem Permissions

> **This material directly adapted from Prof. Aviv's [material](https://classes.adamaviv.com/ic221/s18/units/01/unit.html#org26c0ea7).**

We've seen how the filesystem is used to store and name files and channels, and we've seen the APIs to access directories and files.
If everyone had access to every else's files, then there would be no way to secure your information.
Thus, systems provide means to control different user's *access* to different files and directories.

To understand the core concept of *access control* in UNIX, we have to understand

1. how UNIX thinks, implements, and organizes users,
2. how files and directories provide various access permissions to different users and how to alter those permissions, and
3. how services, even when executed by a user, can have heightened access to system resources.

## File and Directory Permissions

Lets first remind ourselves of the properties of a file that are returned by running `ls -l`:

```
.- Directory?
|    .-------Permissions                   .- Directory Name
| ___|___     .----- Owner                 |
v/       \    V     ,---- Group            V
drwxr-x--x 4 aviv scs 4096 Dec 17 15:14 imma_directory
-rw------- 1 aviv scs 400  Dec 19  2013 .ssh/id_rsa.pub
                       ^   \__________/    ^
File Size -------------'       |           '- File Name
  in bytes                     |
                               |
   Last Modified --------------'
```

An example from the class' server, we create a file, a directory, and see the results:

```
$ echo hi > file.txt
$ mkdir hello
$ ls -l
-rw-r--r-- 1 gparmer dialout    3 Mar 18 12:19 file.txt
drwxr-xr-x 2 gparmer dialout 4096 Mar 18 12:20 hello
```

There are two important parts to this discussion: the owner/group and the permissions.
The owner and the permissions are directly related to each other.
Often permissions are assigned based on user status to the file, either being the owner or part of a group of users who have certain access to the file.

### File Ownership and Groups

The owner of a file is the user that is directly responsible for the file and has special status with respect to the file permission.
Users can also be grouped together in group, a collection of users who posses the same permissions.
A file also has a group designation to specify which permission should apply.

You all are already aware of your username. You use it all the time, and it should be a part of your command prompt.
To have UNIX tell you your username and connection information on this machine, use the command, who am i:

```
$ who
gparmer  pts/1        2022-03-18 12:12 (128.164...)
```

You can use the whoami (no spaces) command to print only your username:

```
$ whoami
gparmer
```

The first part of the output is the username, for me that is aviv, for you it will be your username.
The rest of the information in the output refers to the terminal, the time the terminal was created, and from which host you are connected.

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
Cool.

### The password and group file

Groupings are defined in two places.
The first is a file called /etc/passwd which manages all the users of the system.
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

**Question:**
Take a moment to explore these files and the commands.
See what groups you are in.

### File Permissions

We can now turn our attention to the permission string.
A permission is simply a sequence of 9 bits broken into 3 octets of 3 bits each.
An octet is a base 8 number that goes from 0 to 7, and 3 bits uniquely define an octet since all the numbers between 0 and 7 can be represented in 3 bits.

Within an octet, there are three permission flags, read, write and execute.
These are often referred to by their short hand, r, w, and x.
The setting of a permission to on means that the bit is 1.
Thus for a set of possible permission states, we can uniquely define it by an octal number:

```
rwx -> 1 1 1 -> 7
r-x -> 1 0 1 -> 5
--x -> 0 0 1 -> 1
rw- -> 1 1 0 -> 6
```

A full file permission consists of the octet set in order of user, group, and global permission.

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

These define the permission for the user of the file, what users in the same group of the file, and what everyone else can do.
For a full permission, we can now define it as 3 octal numbers:

```
-rwxrwxrwx -> 111 111 111 -> 7 7 7
-rwxrw-rw- -> 111 110 110 -> 7 6 6
-rwxr-xr-x -> 111 101 101 -> 7 5 5
```

To change a file permission, you use the chmod command and indicate the new permission through the octal.
For example, in part5 directory, there is an executable file hello_world.
Let's try and execute it.
To do so, we insert a `./` in the front to tell the shell to execute the local file.

```
$ gcc helloworld.c -o helloworld
$ chmod 600 hello world
$ ./helloworld
-bash: ./helloworld: Permission denied
```

The shell returns with a permission denied.
That's because the execute bit is not set.

```
$ ls -l helloworld
-rw------- 1 ycombinator ycombinator 16704 Mar 18 13:28 helloworld
```

Let's start by making the file just executable by the user, the permission 700.
And now we can execute the file:

```
$ chmod 700 helloworld
$ ls -l helloworld
-rwx------ 1 ycombinator ycombinator 16704 Mar 18 13:28 helloworld
$ ./helloworld
Hello World!
```

This file can only be executed by the user, not by anyone else because the permissions for the group and the world are still `0`.
To add group and world permission to execute, we use the permission setting `711`:

```
$ chmod 711 helloworld
$ ls -l helloworld
-rwx--x--x 1 aviv scs 7856 Dec 23 13:51 helloworld
```

At times using octets can be cumbersome, for example, when you want to set all the execute or read bits but don't want to calculate the octet.
In those cases you can use shorthands.

- `r`, `w`, `x` shorthands for permission bit read, write and execute
- The `+` indicates to add a permission, as in `+x` or `+w`
- The `-` indicates to remove a permission, as in `-x` or `-w`
- `u`, `g`, `o` shorthands for permission bit user, group, and other
- a shorthand refers to all, applying the permission to user, group, and other

Then we can change the permission

```
chmod +x file   <-- set all the execute bits
chmod a+r file  <-- set the file world readable
chmod -r  file  <-- unset all the read bits
chmod gu+w file <-- set the group and user write bits to true
```

Depending on the situation, either the octets or the shorthands are preferred.

### Changing File Ownership and Group

The last piece of the puzzle is how do we change the ownership and group of a file.
Two commands:

- `chown <user> <file/directory>` - change owner of the file/directory to the user
- `chgrp <group> <file/directory>` - change group of the file to the group

Permission to change the owner of a file is reserved only for the super user for security reasons.
However, changing the group of the file is reserved only for the owner.

```
$ ls -l
total 16
-rwxr-x--- 1 aviv scs 9133 Dec 29 10:39 helloworld
-rw-r----- 1 aviv scs   99 Dec 29 10:39 helloworld.cpp
$ chgrp mids helloworld
$ ls -l
total 16
-rwxr-x--- 1 aviv mids 9133 Dec 29 10:39 helloworld
-rw-r----- 1 aviv scs    99 Dec 29 10:39 helloworld.cpp
```

Note now the hello world program is in the mids group.
I can still execute it because I am the owner:

```
$ ./helloworld
Hello World
```

However if I were to change the owner, to say, `pepin`, we get the following error:

```
$ chown pepin helloworld
chown: changing ownership of ‘helloworld’: Operation not permitted
```

Consider why this might be.
If any user can change the ownership of a file, then they could potentially upgrade or downgrade the permissions of files inadvertently, violating a security requirement.
As such, only the super user, or the administrator, can change ownership settings.

## Security and Programming with Users and Groups

Through this class we have seen a number of security settings provided by the operating system. Formost, we discussed users and groups, file permissions, the terminal login, and finally the concept of system calls generally and how that system is designed to protect the user from itself.

Let's take a moment to review some of these concepts and the O.S. security settings thereof.

### Users and Groups

All users are defined in the /etc/passwd file with lines like such:

```
.-- user name         .-- full name   .--- home directory
|                     |               |
v                     v               v
aviv:x:35001:10120:Adam Aviv {}:/home/scs/aviv:/bin/bash
        ^     ^                                   ^
uid ----'     '--- gid (Default)                  '--- default shell
```

This is my passwd entry. It stores my user name, my user id, my default group id, my full name, my home directory, and my default shell. Every user has a unique username, user id, and default group; however, a user can be assigned to multiple groups. Group information is fined in the /etc/group directory, and here is entry for my default group:

```
.-- group name
|
v
scs:*:10120:webadmin,www-data,lucas,slack
       ^    \___________________________/
gid ---'                |
                        '- Additional users in that group
```

From the command line, the tool id will print this information, as well as groups

```
aviv@saddleback: ~ $ id
uid=35001(aviv) gid=10120(scs) groups=10120(scs),27(sudo),15000(mids)
aviv@saddleback: ~ $ groups
scs sudo mids
```

One thing you might notice is that I am in the sudo group … on this computer, at least. We'll come back to this later.

### Permissions

Access to files are permission-ed based on the user and group designation of the accessing agent. Typically, this is a user, but the same designations are assigned to running processes. The permissions of a file can be observed using ls -l or stat:

```
aviv@saddleback: demo $ ls -l
total 4
-rwxr--r-- 1 aviv scs    0 Mar 27 09:41 a
-rw--wx--- 1 aviv scs    0 Mar 27 09:41 b
-------rwx 1 aviv scs    0 Mar 27 09:41 c
drwxr-x--- 2 aviv scs 4096 Mar 27 09:41 d
aviv@saddleback: demo $ stat b
  File: ‘b’
  Size: 0         	Blocks: 0          IO Block: 524288 regular empty file
Device: 1ch/28d	Inode: 34996239    Links: 1
Access: (0630/-rw--wx---)  Uid: (35001/    aviv)   Gid: (10120/     scs)
Access: 2015-03-27 09:41:22.884094861 -0400
Modify: 2015-03-27 09:41:22.884094861 -0400
Change: 2015-03-27 09:41:41.292753637 -0400
```
And if we look at a particular mode portion, let's recall how the permissions are identified:

```
user   other       .- group
  |     |          |
 .-.   .-.         v
-rwxr--r-- 1 aviv scs    0 Mar 27 09:41 a
^   '-'       ^
|    |        '-- user/owner
|   group
|
'- Directory Bit
```

When an operation is performed on the file, such as reading, writing, or executing, the user taking the action is compared to the permission.
If the right permissions are set, and the user matches those permissions, the action can be taken.
For example, a user that is not aviv may still read the file if they are in group scs.
More, even a user who meets neither criteria, the owner or group of the file, can still read the file because the other read permission is set.

Permissions of files can be changed using three commands:

- `chmod` : change the permissions string of the file which can only be done by the owner of the file (or super user).
- `chgrp` : change the group of the file which can only be done by the owner of the file (or super user).
- `chown` : change the owner of the file which can only be done by the super user.

The super user for unix systems is referred as `root`.
It has full privileges and can do whatever it wants.
Creating multiple levels of permissions by dividing users from one privilege to another is a key security concept.
A key question is how does this occur?
To understand this process, we have to start with when the user logs in.

### Terminal Login and Password Checking

The log procedure is not a huge focus of this lesson; however, what happens after login is incredibly relevant. Recall from the lectures on the tty that when a user "calls" a tty the program getty will execute login. Following the diagram:

```
 runs as root
.......................................
:  .-------.                          :
:  | getty |                          :
:  '-------'                          :
:      |                              :
:    exec() <-------.                 :
:      |            |                 :
: (1)  v            | (failed)        :
:  .--------.       |     ............:
:  | login  | ------'     :
:  '--------'             :                    runs as the user
:      | (success)        :   ..................................
:      |      ............:   :                                :
:      |      :   ............: .-------. (3)                  :
:     fork() -:---:- exec() --> | shell |                      :
:.............: ^ :             '-------'                      :
 (2)            | :                  |                  .----. :
  changes ______| :               fork() --- exec() --> | ls | :
    user          :                                     '----' :
                  :............................................:
```

At (1), the log procedure is going to authentic a user by asking for a password.
While it would seem logical for passwords to be stored in /etc/passwd, it isn't.
The reason /etc/passwd is named such is that it used to store password; however, that is no longer the case.
Now passwords are stored are in the file /etc/shadow which is carefully protected, and passwords are not stored in plain text in /etc/shadow but rather stored using secure hashes.

Of more interest to this lesson is what happens if the user successfully logs into the system.
The log in procedure needs to run in a privileged state so that passwords can be checked from /etc/shadow.
Only the root user has access to read/write the file, but we do not want it to be the case that when the user's shell starts up he/she gets the same permissions as the root user.
To prevent that, there is a deescalation of privilege level by setting the effective user of the shell program to the user the that just logged onto the system – occurring at step (2).
By the time the shell executes commands, the user is running — occurring at step (3).

### Users/Group Capabilities of Programs

Running programs inherit the permissions of the user who execute it, but we will see how that might change.
To start, let us first look at the system programming constructs for observing and testing the users permission, and the errors associated with permission.
Following, we can look at how to escalate or change the permission settings.

#### Observing the privilege settings of programs

There are two basic system calls for retrieving useer and group information for an execution program.

- `uid_t getuid(void)` - Returns the real user id of the calling process.
- `gid_t getgid(void)` - Returns the real group id of the calling process.

Let's look at an example program.

```c
/*get_uidgid.c*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]){

  uid_t uid;
  gid_t gid;

  uid = getuid();

  gid = getgid();

  printf("uid=%d gid=%d\n", uid, gid);

}
```

When executing, my user id and group id is printed to the terminal:

```
m179998@saddleback: git $ ~aviv/lec-23-demo/get_uidgid
uid=179998 gid=15000
```

These values are mine.
If you were to run the same program, you would get a different value.
For example, I have a test student account m17998 and if I run this program as that user:

```
m159998@saddleback: git $ ~aviv/lec-23-demo/get_uidgid
uid=35013 gid=15000
m159998@saddleback: git $ ls -l ~aviv/lec-23-demo/get_uidgid
-rwxr-x--x 1 aviv scs 8622 Mar 30 10:40 /home/scs/aviv/lec-23-demo/get_uidgid
```

Even though the program is owned by the user aviv, the execution of the program as the process takes on the permissions of the user that runs the program, which is m179998 in this demo.
However, this could be changed.

#### Extra permission modes for set-user-id/set-group-id

There is also an obvious need to be able to set the user/group privileges of a running program.
For example, consider the submission system we have been using all semester.
You all run a program in my home directory:

```
~aviv/bin/ic221-submit
```

This program, run by you, has your user and group permissions, but it is able to take your submission and copy/save those submissions to my home directory, with my permissions at a location where you do not have access to write.
How is that possible?

If you could somehow change the user/group setting of the running program to my user/group priviledge, then you could write to my home directory.
This is essentially how the submission system works.

To have a program run with a different user's capabilities requires additional permissions on the program beyond just that the user can execute the program.
These permissions are called the set-bit and if we look at the man page for chmod, the set-bit is composed of three permission bits we previously ignored:

A numeric mode is from one to four octal digits (0-7), derived by adding up the bits with values 4, 2, and 1.
Omitted digits are assumed to be leading zeros.
*The first digit selects the set user ID (4) and set group ID (2) and restricted deletion or sticky (1) attributes.*
The second digit selects permissions for the user who owns the file: read (4), write (2), and execute (1); the third selects permissions for other users in the file's group, with the same values; and the fourth for other users not in the file's group, with the same values.

That is, we previously assumed a permission string contained 3 octal digits, but really there are 4 octal digits.
The missing octal digit is that for the set-bits.
There are three possible set-bit settings and they are combined in the same way as other permissions:

- `4` or `s+u` - set-user-id : sets the program's effective user id to the owner of the program
- `2` or `s+g`- set-group-id : sets the program's effective group id to the group of the program
- `1` or `t` - the sticky bit : used to denote the memory loading of a program or directory

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

When we look at the ls -l output of the program, the permission string reflects these settings with an "s" in the execute part of the string for user and group.

```
aviv@saddleback: lec-23-demo $ ls -l get_uidgid
-rwsr-s--x 1 aviv scs 8778 Mar 30 16:45 get_uidgid
```

#### Real vs. Effective Capabilities

With the set-bits, when the program runs, the capabilities of the program are effectively that of the owner and group of the program.
However, the real user id and real group id remain that of the user who ran the program.
This brings up the concept of effective vs. real identifiers:

- *real user id* (or *group id*) : the identifier of the actual user who executed a program
- *effective user id* (or *group id*) : the idenifier for the capabilities or permissions settings of an executing program.

The system calls getuid() and getgid() return the real user and group identifiers, but we can also retrieve the effective user and group identifiers:

- `uid_t geteuid(void)` - return the effective user identifier for the calling process
- `gid_t getegid(void)` - return the effective group identifer for the calling process

We now have enough to test set-bit programs using a the following program that prints both the real and effective user/group identities.

```c
/*get_euidegid.c*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char * argv[]){

  uid_t uid,euid;
  gid_t gid,egid;

  uid = getuid();
  gid = getgid();
  printf(" uid=%d  gid=%d\n",  uid,  gid);

  euid = geteuid();
  egid = getegid();
  printf("euid=%d egid=%d\n", euid, egid);
}
```

As the owner of the file, after compilation, the permissions can be set to add set-user-id:

```
aviv@saddleback: lec-23-demo $ make get_euidegid
cc     get_euidegid.c   -o get_euidegid
aviv@saddleback: lec-23-demo $ chmod u+s get_euidegid
aviv@saddleback: lec-23-demo $ ls -l get_euidegid
-rwsr-x--x 1 aviv scs 8730 Mar 31 08:31 get_euidegid
```

Now as the m179998 user, the program can be run, and we see that the effective user id of the program is aviv's id:

```
m179998@saddleback: ~ $ ~aviv/lec-23-demo/get_euidegid
 uid=179998  gid=15000
euid=35001 egid=15000
m179998@saddleback: ~ $ id
uid=179998(m179998) gid=15000(mids) groups=15000(mids),15001(ic221)
m179998@saddleback: ~ $ id aviv
uid=35001(aviv) gid=10120(scs) groups=27(sudo),15000(mids),10120(scs),15001(ic221)
```

Continuing the example, we can see the other set-bit settings give different effective user/group settings:

```
aviv@saddleback: lec-23-demo $ chmod g+s get_euidegid
aviv@saddleback: lec-23-demo $ ls -l get_euidegid
-rwsr-s--x 1 aviv scs 8730 Mar 31 08:31 get_euidegid

m179998@saddleback: ~ $ ~aviv/lec-23-demo/get_euidegid
 uid=179998  gid=15000
euid=35001 egid=10120

aviv@saddleback: lec-23-demo $ ls -l get_euidegid
-rwxr-s--x 1 aviv scs 8730 Mar 31 08:31 get_euidegid

m179998@saddleback: ~ $ ~aviv/lec-23-demo/get_euidegid
 uid=179998  gid=15000
euid=179998 egid=10120
```

And just to show how the effective user ID plays a role, let's consider what happens when we have set-group-id program that opens a file:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int
main(int argc, char * argv[]){

  int i,fd;

  for(i=0;i<argc;i++){

    //create an empty file
    if( (fd = open(argv[i],O_CREAT,0666) > 0) ){
      close(fd);
    }else{
      perror("open");
    }
  }

  return 0;
}
```

When we compile we can set this program to set-group-id:

```
aviv@saddleback: lec-23-demo $ make create_file
cc     create_file.c   -o create_file
aviv@saddleback: lec-23-demo $ chmod g+s create_file
aviv@saddleback: lec-23-demo $ ls -l create_file
-rwxr-s--x 1 aviv scs 8620 Mar 31 08:41 create_file
```

Now, let's create a file as the m179998 user:

```
m179998@saddleback: ~ $ ~aviv/lec-23-demo/create_file a b c
m179998@saddleback: ~ $ ls -l a b c
-rw-r----- 1 m179998 scs 0 Mar 31 08:42 a
-rw-r----- 1 m179998 scs 0 Mar 31 08:42 b
-rw-r----- 1 m179998 scs 0 Mar 31 08:42 c
```

Notice that the group of the file is scs not mids, which is the default group.

However, see what happens if we make this program set-user-id instead:

```
aviv@saddleback: lec-23-demo $ chmod g-s create_file
aviv@saddleback: lec-23-demo $ chmod u+s create_file
aviv@saddleback: lec-23-demo $ ls -l create_file
-rwsr-x--x 1 aviv scs 8620 Mar 31 08:41 create_file
```

```
m179998@saddleback: ~ $ rm -f a b c
m179998@saddleback: ~ $ ~aviv/lec-23-demo/create_file a b c
open: Permission denied
open: Permission denied
open: Permission denied
m179998@saddleback: ~ $ ls -l a b c
ls: cannot access a: No such file or directory
ls: cannot access b: No such file or directory
ls: cannot access c: No such file or directory
```

The operation is not permitted, and this is because the user aviv does not have the permission to write to the directory.
But, what if we wanted to create a file in the director that is owned by aviv in a directory that the real user has permission to write to? What we need is a way to programmatically change the capabilities.

#### Programmatically Downgrading/Upgrading Capabilities

The set-bits automatically start a program with the effective user or group id set; however, there are times when we might want to downgrade priviledge or change permission dynamically. There are two system calls to change user/group settings of an executing process:

- `setuid(uid_t uid)` - change the effective user id of a process to uid
  `setgid(gid_t gid)` - change the effective group id of a proces to gid

The requirements of `setuid()` (for all users other than root) is that the effective user id can be changed to the real user id of the program or to an effective user id as described in the set-bits.
The root user, however, can downgrade to any user id and upgrade back to the root user.
For `setgid()` the user can chance the group id to any group the user belongs to or as allowed by the set-group-id bit.

Now we can look at a program that downgrades and upgrades a program dynamically:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main()
{
  uid_t uid,euid;
  gid_t gid,egid;

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

If we look at the output with the program set-user-id ran by m179998:

```
m179998@saddleback: ~ $ ~aviv/lec-23-demo/setuid
 uid=179998  gid=15000
euid=35001 egid=15000
---- setuid(179998) ----
 uid=179998  gid=15000
euid=179998 egid=15000
---- setuid(35001) ----
 uid=179998  gid=15000
euid=35001 egid=15000
```

### `sudo` and `su`

With this understanding of how user and group id are assigned, we can turn our attention to two built in commands that perform these actions.
In particular, sudo and su which will execute a command as a specified user or switch to a specified user.
By default, these commands execute as the root user, and you need to know the root password or have sudo access to use them.

We can see this as the case if I were to run the `get_euidegid` program using sudo.
First notice that it is no longer set-group or set-user:

```
aviv@saddleback: lec-23-demo $ ls -l get_euidegid
-rwxr-x--x 1 aviv scs 8730 Mar 31 08:31 get_euidegid
aviv@saddleback: lec-23-demo $ sudo ./get_euidegid
[sudo] password for aviv:
 uid=0  gid=0
euid=0 egid=0
```

After sudo authenticated me, the program's effective and real user identification becomes 0 which is the uid/gid for the root user.

#### sudoers

Who has permission to run sudo commands?
This is important because on many modern unix systems, like ubuntu, there is no default root password.
Instead certain users are deemed to be sudoers or privileged users.
These are set in a special configuraiton file called the `/etc/sudoers`.

```
aviv@saddleback: lec-23-demo $ cat /etc/sudoers
cat: /etc/sudoers: Permission denied
aviv@saddleback: lec-23-demo $ sudo cat /etc/sudo
sudoers    sudoers.d/
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

Notice that only root has access to read this file, and since I am a sudoer on saddleback I can get access to it. If you look carefully, you can perform a basic parse of the settings. The root user has full sudo permissions, and other sudoer's are determine based on group membership. Users in the sudo or admin group may run commands as root, and I am a member of the sudo group:

```
aviv@saddleback: lec-23-demo $ id
uid=35001(aviv) gid=10120(scs) groups=10120(scs),27(sudo),15000(mids),15001(ic221)
```

However, on a lab machine, I do not have such group settings:

```
aviv@mich302csd01u: ~ $ id
uid=35001(aviv) gid=10120(scs) groups=10120(scs),15000(mids)
```
