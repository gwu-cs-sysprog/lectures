---
title: "UNIX Personalities: Containers"
author: "Gabe Parmer 👋, research by Sean McBride"
---

#  UNIX's World View

## Users

- System includes all users
- All users can see each other (`who`)
- All processes are visible (`ps aux`)

## Filesystem & Access Control

- All users share the same filesystem
- Permissions/ownership control which files are accessible to which users
- `path` $\to$ file/directory

## Dependency Management

Dependencies

- Dynamic libraries (which version?, is it installed?)
- Services (databases, webservers, GUI frameworks)

## `root`

- Universal access
-

# Containers World View

## Application Development History$^*$

90s - bespoke, targeted development

- Compile everything together
- Static libraries, or dynamic libraries for an OS version

00s - frameworks, and increased application complexity

- Webserver/DB/Business logic
- *Frameworks* w/ library/language dependencies
- "dependency hell"
- Growth of *VMs as unit of "application"*

## Application Development History II

10s - Applications via composition

- Coordination between components
- App development via library/service composition
- Languages with package management (but C dependencies)
- Growth of *Containers as unit of "application"*
- DevOps curate dev vs. production

Now - Assumed containers for distribution

- Containers + Kubernetes
- DevOps job of developers
- NP-complete dependency resolution in languages

## App = Program + Dependencies

## Separate System Namespaces

# UNIX Namespaces

## Namespace Definition

> A **Name** is often a string or integer used to uniquely *identify or address* a resource.
>
> A **Namespace** is the set of these **Names**.

Name Examples:

- filesystem *path*s
- process *pid*s
- internet URLs

## UNIX [Namespaces](https://en.wikipedia.org/wiki/Linux_namespaces)

What are UNIX *namespaces*? In Linux: read more in `man namespaces`.

- `pid_namespaces(7)` - **Process IDs**
- `user_namespaces(7)` - **User IDs/Group IDs**
- `mount_namespaces(7)` - **Filesystem namespaces**
- `ipc_namespaces(7)` - System V IPC/POSIX message queue IDs
- `network_namespaces(7)` - Networking identity and addresses
- `uts_namespaces(7)` - System identity (`hostname`)

The [`unshare` system call](https://www.kernel.org/doc/html/latest/userspace-api/unshare.html) unshares namespaces in the next forked child.

## `pid` Example

Containers can have overlapping `pid`s, and cannot address other container's `pid`s!

```
$ whoami
root
$ kill -9 1234
```

...cannot kill `1234` if it is in another container!

```
unshare(CLONE_NEWPID);
if (fork() == 0) {
	assert(getpid() == 1);
	execv("/bin/init");
}
```

## User Example

- Each container can have a root!
- User `1234` in one container is not the same user in another!

```
unshare(CLONE_NEWUSER);
if (fork() == 0) {
	setuid(0); /* I'm root now!...in my own little domain */
	assert(getuid() == 0);
	execv("/bin/init");
}
```

## Filesystem Example

> `chroot(path)` - Set the `/` of the FS *for this process* to be `path`.

Now all files I can access are in the *subdirectory* `path`.

```
chroot("/home/gparmer/myroot/");
chdir("/");
unshare(CLONE_NEWPID | CLONE_NEWUSER);
if (fork() == 0) {
	/* assuming `init` was originally in /home/gparmer/myroot/bin/init */
	execv("/bin/init");
}
```

Linux uses [`pivot_root(new_root, old_path)` (link)](https://man7.org/linux/man-pages/man2/pivot_root.2.html) instead of `chroot` [(why)](https://lists.linuxcontainers.org/pipermail/lxc-devel/2011-September/002065.html).
See the [code](https://github.com/opencontainers/runc/blob/main/libcontainer/rootfs_linux.go#L840-L842) in Docker.

# Docker Service

## Docker is a service

![Docker service receives requests to manipulate containers. Credit: https://docs.docker.com](figures/containers/docker_architecture.svg)

## Docker Orchestration

Service (`daemon`) created by `systemd`

- `dockerd`
- awaits commands on a domain socket @ `/var/run/docker.sock`

	- start new containers with a specification
	- terminate containers
	- check in on a container

# Containers Summary

## Containers and Android

| Goal             | Containers                             | Android                          |
|------------------|----------------------------------------|----------------------------------|
| Namespace Mgmt.  | Separate *all* namespaces              | Intents for shared services      |
| App Isolation    | Namespace separation = isolation       | `uid` separation                 |
| Dependency Mgmt. | Container includes set of dependencies | App compiled for Android version |
