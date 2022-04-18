I think that the approach for the container/android discussions can be 1, or a little more bit than 1 days (1:15 to 1:35) discussing what the technology is, and then 1 day, or a little less than a day, talking about the underlying APIs. Might be that containers are 3 days (3x 1:15) as it mighht be easier to dig up information about them. Demos might go in there somewhere (for containers). For example, for containers:

## Brainstorm: is UNIX enough? Strong enough isolation? Useful enough configurability?

## Traditional UNIX Security Controls / Isolation

UNIX was built as a timesharing operating system. This assumed a simple model of users logging into a system via serial or remote terminals and interact with the shell to perform job control and execute programs in the foreground and background.

Security controls are based around users and groups identified by UIDs and GIDs respectively.

Normal users own their own `home` directory, but have limited privileges elsewhere.

A special user called the `superuser` has administrator privileges to all files in a filesystem. This is typically `root`. Users typically switch to administrator only when required via "switch user" `su` or "switch user and do" `sudo`, which runs a single command as another user (typically the superuser). System daemons and critical userspace processes such as `init` are owned by the `root` user.

This interacts with the filesystem. Each file's `inode` structure have user and group ownership and a set of mode bits that control whether a user can read, write or execute a file (or directory) on the filesystem. This can be used to hide parts of a filesystem, keep user's home directories private, and limit sensitive system state and behavior to administrators.

The protection domain and schedulable entity is the `process`.

A process has both a `real user ID` and an `effective user ID` (stored in the u area of the kernel's process table). The real UID is the user responsible for the process. The effective UID is the user ID that is used by syscalls and permissions checks:

- Set as the owner of new files that get created
- Used when checking file access permissions
- Used by `kill` to control how signals are sent.

Similarly, a process has both real and effective GIDs.

The syscall `setuid` is used to change the effective user id.
The syscall `setgid` is used to change the effective group id.

The file mode bits mentioned above have special bits that only apply to executables. The `setuid` bit causes a process to set its effective user to its owning user regardless of who runs it. The `setgid` bit causes a process to get its effective group ID to its owning group regardless of the group of the user that ran it.

A traditional use of the `setuid` bit is in the `passwd` binary, which is used to allow a user to set his/her own password in the `/etc/passwd` database. Both the `passwd` binary and `/etc/passwd` database are owned by the root user. However, because `setuid` is set, the `passwd` process has an effective user id of `root`, which allows it to open the `/etc/passwd` database. The `passwd` then uses the "real user ID" to determine which password to change. The `setuid` bit is frequently exploited by hackers to execute a shell as root.

A process also stores information about its view of the filesystem in the kernel's process table (in the u area), including:

- current directory - the inode of the process' current directory in the filesystem. This is used to resolve relative paths. Set by the syscall `chdir`.
- root directory - the inode of the process' current root in the filesystem. If set, this shadows the global filesystem root. This is used to resolve absolute paths. Set by the syscall `chroot`.

The `namei` algorithm is responsible for interpreting paths. It uses the process' root, current directory, and the file permissions of inodes on the filesystem to hide information the user is not permitted to see. It enforces "chroot jails"

References:
Bell Labs Patent for Unix Data Protection Model: https://worldwide.espacenet.com/patent/search/family/023489738/publication/US4135240A?q=pn%3DUS4135240

## Traditional UNIX Resource Controls

Traditional Unix allows processes to be restricted via `setrlimit`. This can be used to cap the amount of system resources that a process is able to use. Threads share the resources limits of their process.

Forked processes inherit the same limits as their parents.

In order to achieve something like delegation, a multiprocess program can `getrlimit`, divide by the number of child processes, and then use `setrlimit` after `fork` but before `exec`.

There do not seem to be any concept of resouce limits for higher level UNIX concepts like `process groups` or `sessions`.

`ulimit` is a shell builtin or utility that sets resource limits on a user's shell. Because forked processes inherit these limits, this limits apply to all programs that the user runs from shell.

## Challenges

- Superuser versus normal user is "all or nothing." Unable to delegate a subset of administrative capabilities globally across all files.
- Security policy is implicit in file and user management.
- Groups can somewhat be used to approximate "roles"
- Unix assumes timesharing style interactive computing where programs execute under the direct control of an interactive user. Unix Daemons feel like a bit of a kludge. Unix scaled up to Big Iron. It now is mostly used as a server running daemons listening on a network rather than for multi-user timesharing. How to handle "hosting" and multi-tenancy with SLAs?
- Changing security policy can require touching a lot of files.

## V7 -> FreeBSD Jails, Solaris Zones, etc.

During the Unix wars, vendors attempts to improve isolation between daemons, harden against malicious Internet users, and better support multi-tenancy.

FreeBSD Jails and Solaris Zones were the two main techniques. They built on the idea of `chroot` jails.

## Containers

_Containers, what is this?_

A container is not a lightweight VM. It's chroot on steroids

A container "feels like a VM" because it has its own:

- has own process space
- has own network interface
- can run as root
- can install packages
- can run services
- can locally mess us routing, iptables, etc.

A container differs from a VM because:

- It uses the host kernel, so it can't boot into a different OS or load special kernel modules
- It doesn't load init as PID 1

Containers are subtrees of the Unix process hierarchy that are configured to be fully encapsulated such that they look like an entirely new instance of the system. This is conceptually taking the ideas of a `chroot jail` and applying them across all syscalls and interfaces such that the Linux process cannot see or affect processes outside of the container boundry.

Typically, container is synonymous with "Linux container," as the technologies that containerization depends upon are part of the Linux kernel. However, the approach can be generalized and has been backported to the Windows NT kernel for example.

## Namespaces

namespaces limit what you can see. You can't use what you can't see!

_What are the namespaces in UNIX? Enumerate and background._

I don't belive that traditional Unix provided namespacing beyong the ability to set the absolute root of a process via `chroot()`.

I suppose other namespaces are things that aren't expressed on the filesystem:
Process IDs
User IDs
Other Idenfiers?
System V IPC IDs?
POSIX MQ IDs?

It seems that the idea of per-process namespaces comes from Plan 9

"The various services being used by a process are gathered together into the process's name space, a single rooted hierarchy of file names. When a process forks, the child process shares the name space with the parent. Several system calls manipulate name spaces. Given a file descriptor fd that holds an open communications channel to a service, the call
mount(int fd, char \*old, int flags)"
Source: https://web.archive.org/web/20140906153815/http://www.cs.bell-labs.com/sys/doc/names.html

This seems to be a natural outgrowth of choosing to represent all system resources as a hierarchical filesystem.

The namespace of each system resource essentially becomes a file hierarchy, and the namespaces can be defined by changing the root node and limiting a process to a subtree of resources. Each of these namespaces can be mounted into a single hierachical view using the selected roots.

## Containers look like an entirely new instance of the system, can't see other applications or users.

## Separate filesystem namespaces - what does this mean for isolation? what does it mean for customizability? dynamic library and binary customization.

## Separate pid namespaces - what does this mean for isolation?

Types of Linux Namespaces

pid_namespaces(7) - Isolate the Process PID number space

- The pid namespace generates a new set of a Process IDs for all processes within this namespace.
- Because processes inherit the namespaces of those that they are forked from, pid_namespaces are effectively subtrees starting at a particular root.
- The root of the namespace is numbered `PID 1` is treated like `init`, inheriting orphan processes within its own namespace as children.
- The first pid namespace is rooted by the actual host `init` and includes all processes on the host, including those that are in their own PID namespaces.
- The PIDs are always relative to a namespace, so a process might be PID 1 in namespace 2 and PID 23 in namespace 1.
- PID namespaces for a hierarchy. Parents include unique IDs for the processes of all child PID namespaces. Namespaces cannot see the processes of ancestors.

cgroup_namespaces(7) Cgroup root directory

- namespaces cgroup definitions?
- This one is a bit hazy. I presume these nest if the root node is in a ancestor cgroup?

ipc_namespaces(7) System V IPC, POSIX message queues

- Namespaces the BSD / System V IPC calls in POSIX.

network_namespaces(7) Network devices, stacks, ports, etc.

- Defines local network interfaces, routing tables, iptables rules, sockets
- By default, it standalot and fully disconnected and standalone
- A interfaces can only be a member of a single namespace, interfaces can be passed between net namespaces
- A bridged network between containers can be build by defining a virtual switch and pairs of virtual NICs in a parent namespace and then passing one half of the veth pairs into each child net namespace

mount_namespaces(7) Mount points

- Isolates the mount points of all devices mounted into the hierarchical filesystem
- Mounts can be either private or shared between containers
- clone() copies the parent's mount_namespace. `mount` and `unmount` then only affects the local namespaces
- TODO: pivot root? chroot? Unclear what needs to be done to a cloned mount namespace to isolate it

user_namespaces(7) User and group IDs

- Allows a user to be UID 0 inside the container, but not on the host!

uts_namespaces(7) Hostname and NIS domain name

## Control Groups (Cgroups):

- Responsible for accounting for and limiting resources across a group of processes. This is somewhat similar to the role of `setrlimit` in traditional Unix.
- A cgroup is

- Defines one or more sets of processes. Each set is a "cgroup"

Linux implements Control Groups as a single tree data structure composed of individual nodes called cgroups. By default, the tree has a single cgroup node containing all processes on the system.

Each process on the system belongs to one and only one cgroup. By default, child processes inherit the cgroup membership of their parents.

Processes can be migrated between cgroups. All threads of a process are always kept in the same cgroup.

cgroup controllers can be attached empty cgroups. These controllers are responsible for metering and limiting resources of a particular type for all of a node's immediate children.

Because controllers can only be attached to cgroups that do not contain processes, this means that processes are only ever in:

- leaf nodes of the tree
- the root node

A controller can only be attached to a cgroup if this controller is attached to all ancestor cgroups up to and including the root node.

It is controlled via a filesystem that is typically mounted in `/sys`.

A less-privileged user can be granted write access to a subtree of the control group. This enables delegation of a portion of system resources. Within its subtree, this user can create child cgroups, further divide resources, move processes, and delegate to even less-privileged users.

Migration of processes across cgroups is discourage. It

Types of Resource Delegation:

- Weights: Each child is delegated a fraction of its parent's resource equal to is own weight divided by the total weight of itself and all siblings
- Limits: A cap on how much of a total resource a child can use. Can be overcommitted, meaning the sum of all children's limits might exceed the parent.
- Protection: A floor for the least amount of a resource available to a cgroup. Might be either hard guarantee or best effort depending on controller / if resources are over-committed
- Allocation: An absolute share of a finite resource.

By default composed of a single node

- Metering and limiting on resources used by processes. CPU, Memory, Block and networking I/O
- Each type of resource is an independent "subsystem" which is structured as a hierarchy (tree).
  - Each node is a group of processes that share the same resources
  - Default is a single root node containing all processes
- Memory cgroups
  - Each group has an active and an inactive set of pages. The inactive set contains candidates for eviction.
  - Accounting is an optional feature that adds page counts for each type of memory pages (file, anonymous) to each group of processes. This reflects the chargeback to each group. If multiple groups use a shared page, only one group "pays"
  - Limits can be set against these counters
    - Limits can be set for physical memory, kernel memory, or total memory
    - An optional soft limit influences how the kernel reclaims memory
    - An optional hard limit triggers a per-group OOM killer. This constrains the OOM killer to a particular group
    - A hard limit can instead trigger user-defined OOM behavior (such as container migration to another host, provisioning additional memory on host, etc.) via oom-notifier.
- HugeTLB cgroup
  - Controls the number of huge pages that can be used by a process
- CPU cgroup
  - Distributes CPU cycles via Weight and Absolute Bandwidth Limit for the normal scheduler and absolute bandwidth allocation for realtime scheduling
  -
  - Track CPU usage on the granularity of a whole cgroup. Better than per-process because many subsystems are multi-process.
  - user/system CPU time
  - Can set weights, but not limits
- Cpuset cgroup
  - Can pin a group to a cpuset of one or more cores
- Blkio cgroup

  - Keep track of (async, sync) x (read, write) per block device
  - Can set limits for a group
  - Can set relative weights to give groups priority

- Net_cls cgroup
  - Assigns outgoing network traffic from a group to a class'
  - Works with "tc" traffic control to shape traffic
- net_prio cgroup
  - Assigns outgoing network traffic from a group a priority. Used by "queueing disciplines"
- device cgroup
  - read/write/mknod permissions per device nodes
- Freeze cgroup
  - Freezes / thaws processes in cgroup
  - Acts similar to SIGSTOP/SIGCONT

A process can be migrated between control groups. Where does this get placed on the tree? What is its parent process?

Systemd wraps this lower level interface.

## Capabilities

- Provides finer grained access than root/non-root
- A root user can be created with limited capabilities

## Copy-on-write, File-system layers, and Union FS

Copy-on-write storage:

Enables Docker Images and Dockerhub

TODO

## Namespace APIs, Cgroup APIs, and Containers by Hand Demo

Kernel Docs for cgroups v2
https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/Documentation/cgroup-v2.txt?id=v4.5&id2=v4.4

RHEL docs on cgroups v2:
https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/setting-limits-for-applications_managing-monitoring-and-updating-the-kernel#understanding-control-groups_setting-limits-for-applications
https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/using-cgroups-v2-to-control-distribution-of-cpu-time-for-applicati'ons_managing-monitoring-and-updating-the-kernel

Overview of many aspects
https://pierrchen.blogspot.com/2018/08/understand-container-index.html

## Docker Specification and Dockerfile YAML:

TODO

## Docker Daemon / Service Architecture:

Note: Docker is not the only option. There are also LXC, runC, systemd-nspawn, etc.

TODO

Docker service (they know domain sockets, and IPC communication with services) to use container APIs.

# Android Differences from Linux

Gabe Notes:

Android, what is?
Services
Per-application users to focus on per-application isolation. Why? How's the FS laid out for this?
What is a user, then? essentially instantiate user execution using services (e.g. GUI).
Matrix of what users and applications can access in UNIX, what user's and applications can access in Android.
Survey of services, and how they think about protection (dont' know what I'm talking about here).
Happy to brainstorm. Just wanted to get something down on "paper"

## Android, what is?

Android is a operating system that builds upon the Linux kernel. It targets phones, wearables, TVs, and cars. It also is supported as a personality on ChromeOS.

However, many would not describe Android as a Linux system because it is not able to run most Linux applications.

Android extensively uses a lightweight CORBA-like RPC framework called Binder, which is used to "run an object-oriented operating system on top of a general-purpose operating system." Binder internally uses shared memory to minimize data copying and the process of each Android app runs a Binder thread pool. A caller marshalls the source data into "parcels", copies the parcels into shared memory mapped in the callee's address space, and then unmarshalls into what the callee expects.

A Binder IPC call can be "two way," which synchronously blocks the caller until a result is returned from the callee or "one way," which is asynchronous and nonblocking. The callee might execute a "callback" by invoking a "one way" call in the opposite direction with the resulting data.

This "object-oriented operating system" is composed of a bunch of System Services that communicate via Binder RPC calls. Developers express the logic of marshalling and unmarshalling complex java class instances via Android Interface Definition Language (AIDL).

Android also abstracts the native hardware interfaces of the Linux kernel, such as `/dev`, via Hardware Abstraction layer (HAL) modules that also communicate via Binder RPC calls. HAL modules are \*.so dynamic libraries that run in userspace. A key objective of HAL modules to allow Android manufacturers to keep some of their drivers as proprietary code and prevent device drivers from getting contaminated by the GPL.

Because of this architecture, most aspects of the Android Framework can conceivably be ported to an alternative kernel, such as Fuchsia, in the future.

Android user space components are generally open-source replacements for free software.

- A BSD licensed libc named Bionic in place of glibc
- A BSD licensed Busybox alternative named Toolbox is used

Android has a custom `init` process that starts several traditional Linux daemons. The init process can optionally start things such as adbd (the android debugger) based on the contents of the configuration file `init.rc`. The `init` process then forks the Zygote process.

The Zygote process is a special process that acts as the parent process of all System Services and all Android apps. It preloads all Android Runtime classes into memory, and then listens for requests to start an app over a domain socket. Requests to this socket causes the Zygote to fork itself, configure the uid/gid of the requested app, and then exec the binary of the app. Via copy-on-write, this causes android apps to effectively share a single copy of the Android Runtime classes. The Zygote process forks and execs the SystemServer process, which in turns forms and execs each of the Android System Services (WindowManagerService, PackageManager, Service, CameraService, ActivityManagerService, etc.)

Note: Android doesn't prepend GC mark bits in front of objects because this would trigger writes and break the benefit of COW from a forked zygote. GC mark bits are kept separate from the rest of heap memory.

Out of these, the ActivityManagerService sends an intent CATEGORY_HOME which causes the launcher app to be started, rendering the home screen.

Android Apps are very different from traditional ELF binaries. There isn't a single entry point, and Android app is actually a distributed system composed of different Android Framework components that communicate using Binder RPC calls. Different components might be called by the Android Framework for different reasons. Components are started on demand and automatically killed based on memory pressure.

Android Apps are composed of the following:

- Activities ~ UI Elements
- Services ~ Daemons
- Content Providers ~ Databases
- Broadcast Receivers ~ Interrupt handlers for intents

Android App services can communicate with each other and with Android System Services via binder if the App includes Android Interface Definition Language (AIDL). The difference between an App Service and a System Service is that a System Service runs from boot to shutdown and an App Service is lifecycle managed, so might start and stop based on usage.

Android Apps are typically written in JVM-languages, such as Kotlin or Java, and compiled through a somewhat complex pipeline. The source code is compiled in Java bytecode, which is recompiled into DEX bytecode, which is AOT compiled into a native binary. Android apps are packaged as APK bundles. These bundles ship Dex ByteCode and AOT-compiled native binaries. The native binary is used for the initial run, and in order to provide fast startup, and this is replaced by OpenJDK-based Just-in-Time compilation after an initial period of code profiling.

Although an Android app is composed of different independent components, all of these components are built, versioned, and published as a single APK package. This means that all that a device needs to execute is a version of the Android Framework at the required API level and the APK. This avoids the common OS pitfall of "DLL Hell," where installed a piece of software requires dependencies to be manually installed beforehand. In an extreme case of this, when using the Android Native Developer Kit (NDK) to call native C or C++ code, \*.so files are embedded within the APK.

The components of an Android App start based on "Intents" mediated the Android Framework and stop automatically based on memory usage.

The ActivityManager system service is what actually does this mediation. It distatches intents and starts/stops activites and services according to the Android App Lifecycle. This is analogous to a kernel scheduler, but instead of dealing with processes, it deals with "Apps" according to the Android App lifecycle. The ActivityManger starts an app by telling the Zygote to fork with a particular class.

An Intent is a passive object that acts as a message between service components.

For example,

1. User is viewing emails in a Mail app and sees a PDF attachment.
2. User touches the attachment, which emits the "View PDF" Intent
3. The Broadcast receiver for a PDF Reader app handles the intent, opening a PDF Viewer Activity on the screen and rendering the PDF

`/acct/`

Android uses control groups to ensure that foreground Apps have a greater share of CPU.

Apps APKs install to `/data/app/`

App data is stored in the `/data/data/` path

Each application has a directory has a
`/data/data/com.android.email/`

They write databases and files to this directory.

The UID and GID are a unique value in the template `app_N`. I.e. `app_6`

## Per-application users to focus on per-application isolation. Why? How's the FS laid out for this?

Because Android is built on Linux, Android Apps execute as Linux processes. However, Android apps sandbox each app via UID-based discretionary access control (DAC). To accomplish this, Android automatically generates a new unique user ID for each Android App. When this app is started, the underlying Linux process is run using the unique UID. This sandboxes the App and uses normal Linux user permissions to prevent Linux processes from interacting with each other directly. Instead, all coordination between processes is handled above the kernel and mediated via the Android Framework primitives.

Over time, the sandbox has been hardened with Linux technologies:

- SELinux
- A seccomp-bpf filter was added to block all Linux syscalls that are not used by the Android Framework.

Source: https://android-developers.googleblog.com/2017/07/seccomp-filter-in-android-o.html
https://source.android.com/security/app-sandbox

Given that Android Apps are actually a distributed system, this Android App sandbox might actually contain multiple processes, such as when running a service and an activity or when running a native ELF libraries packaged inside the APK.

As with all Linux users, this per-App user has a Linux home directory, which an app uses for local persistent file storage.
Application Home directory

"Native User Space"

`/data`

`/system`

Android doesn't use `/bin` or `/lib` at all.

Android System Services written in Java could be updated and distributed via the Marketplace. However, this was not possible for low level components written in C or C++. Android 10 introduced a new APEX container format to enable APK-like updates of low level system services.

Android References:

https://developer.android.com/guide/platform
https://elinux.org/Android_Architecture

Zygote Process:
https://android.googlesource.com/platform/frameworks/base/+/master/core/java/com/android/internal/os/ZygoteInit.java
https://medium.com/@voodoomio/what-the-zygote-76f852d887d9
https://elinux.org/Android_Zygote_Startup

On Binder / AIDL
https://elinux.org/Android_Binder#:~:text=Binder%20is%20an%20Android%2Dspecific,pass%20the%20arguments%20between%20processes.
https://github.com/opersys/binder-explorer-web
https://www.youtube.com/watch?v=SxJ8xZpSeIw
https://elinux.org/Android_Binder
https://developer.android.com/reference/android/view/inputmethod/InputMethodManager.html
authentication:
https://baiqin-droid1001.medium.com/binder-security-f93265d2ec1b
Binder permissions check on service creation
https://developer.android.com/reference/android/content/Context#startService(android.content.Intent)
https://developer.android.com/training/permissions/restrict-interactions
https://developer.android.com/training/permissions/restrict-interactions#other-permission-checks

Android Internals Workshop for Embedded Devs
https://www.youtube.com/watch?v=KLUXPxxJc5c
(On second video)
https://www.youtube.com/watch?v=LimC0XpeT0k

Dalvik Virtual Machine Internals (Circa 2008):
https://www.youtube.com/watch?v=ptjedOZEXPM

Android Security Internals
https://www.opersys.com/presentations/2019-05-15/android-security-internals-pub/slides-main-190515.html#/
https://source.android.com/security/app-sandbox

Sandbox and IDs
https://pierrchen.blogspot.com/2016/09/an-walk-through-of-android-uidgid-based.html
