
# Processes

Programming is hard.
Really hard.
When you write a program, you have to consider complex logic and the design of esoteric data-structures, all while desperately trying to avoid errors.
It is an exercise that challenges all of our logical, creative, and risk management facilities.
As such, programming is the act of *methodically conquering complexity* with our *ability to abstract*.
We walk on the knives-edge of our own abstractions where we can barely, just barely, make progress and engineer amazing systems.

Imagine if when programming and debugging, we had to consider the actions and faults of *all other* programs running in the system?
If your program crashed because one of your colleagues' program had a bug, how could you make progress?

Luckily, we stand on the abstractions of those that came before us.
A key abstraction provided by systems is that of *isolation* - that one program cannot arbitrarily interfere with the execution of another.
This isolation is a core provision of Operating Systems (OSes), and a core abstraction they provide is the **process**.
At the highest-level, each process can only corrupt its own memory, and a process that goes into an infinite loop cannot prevent another process for executing.

A process has a number of properties including:

- It contains the set of memory that is only accessible to that process.
    This memory includes the code, global data, stack, and dynamically allocated memory in the *heap*.
    Different processes have *disjoint* sets of memory, thus no matter how one process alters its own memory, it won't interfere with the data-structures of another.
- A number of *descriptors* identified by integers that enable the process to access the "resources" of the surrounding system including the file system, networking, and communication with other processes.
    When we `printf`, we interact with the descriptor to output to the terminal.
    The OS prevents processes from accessing and changing resources they shouldn't have access to.
- A set of unique identifiers including a process identifier (`pid`).
- The "current working directory" for the process, analogous to `pwd`.
- A owning user (e.g. `sibin`) for whom the process executes on the behalf of.
- Each process has a *parent* - the process that created it, and that has the ability and responsibility oversee it (like a normal, human parent).
- Arguments to the process often passed on the command line.
- Environment variables that give the process information about the system's configuration.

Throughout the class we'll uncover more and more of these properties, but in this lecture, we'll focus on the lifecycle of a process, and its relationship to its parent.
Processes are the abstraction that provide isolation between users, and between computations, thus it is the core of the *security* of the system.

## History: UNIX, POSIX, and Linux

UNIX is an old system, having its origins in 1969 at Bell Labs when it was created by Ken Thompson and Dennis Ritchie^[C was also created as part of the original UNIX as a necessary tool to implement the OS!].
Time has shown it is an "oldie but goodie" -- as most popular OSes are derived from it (OSX, Linux, Android, ...).
In the OS class, you'll dive into an implementation of UNIX very close to what it was in those early years!
The [original paper](figures/unix.pdf) is striking in how uninteresting it is to most modern readers.
UNIX is so profound that it has defined what is "normal" in most OSes.

> UNIX Philosophy: A core philosophy of UNIX is that applications should not be written as huge monolithic collections of millions of lines of code, and should instead be *composed* of smaller programs, each focusing on "doing one thing well".
> Programs focus on inputting text, and outputting text.
> *Pipelines* of processes take a stream of text, and process on it, process after process to get an output.
> The final program is a composition of these processes composed to together using  pipelines.

In the early years, many different UNIX variants were competing for market-share along-side the likes of DOS, Windows, OS2, etc...
This led to differences between the UNIX variants which prevented programmers from effectively writing programs that would work across the variants.
In response to this, in the late 1980s, POSIX standardized many aspects of UNIX including command-line programs and the standard library APIs.
`man` pages are documentation for what is often part of the POSIX specification.
I'll use the term UNIX throughout the class, but often mean POSIX.
You can think of Linux as an implementation of POSIX and as a variant of UNIX.

UNIX has been taken into many different directions.
Android and iOS layer a mobile runtime on top of UNIX; OSX and Ubuntu layer modern graphics and system  management on top of UNIX, and even Windows Subsystem for Linux (WSL) enables you to run a POSIX environment inside of Windows.
In many ways, UNIX has won.
However, it has won be being adapted to different domains -- you might be a little hard-pressed looking at the APIs for some of those systems to understand that it is UNIX under the hood.
Regardless, in this class, we'll be diving into UNIX and its APIs to better understand the core technology underlying most systems we use.

The core UNIX philosophy endures, but has changed shape.
In addition to the pervasive deployment of UNIX, Python is popular as it is has good support to compose together disparate services and libraries, and applications are frequently compositions of various REST/CRUD webservice APIs, and `json` is the unified language in which data is shared.
