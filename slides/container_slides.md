---
title: "UNIX Personalities: Containers"
author: "Gabe Parmer 👋, research by Sean McBride"
---

#  UNIX's World View

## Users

## Filesystem & Access Control

## Dependency Management

## `root`

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

## `pid` Example

Containers can have overlapping `pid`s, and cannot address other container's `pid`s!

```
$ whoami
root
$ kill -9 1234
```

...cannot kill `1234` if it is in another container!

## User Example

## Filesystem Example

> `chroot(path)` - Set the `/` of the FS *for this process* to be `path`.

Now all files I can access are in the *subdirectory* `path`.

```c
chroot("/home/gparmer/myroot/");
chdir("/");
execv("/bin/init");
```

Linux uses [`pivot_root(new_root, old_path)` (link)](https://man7.org/linux/man-pages/man2/pivot_root.2.html) instead of `chroot` [(why)](https://lists.linuxcontainers.org/pipermail/lxc-devel/2011-September/002065.html).
See the [code](https://github.com/opencontainers/runc/blob/main/libcontainer/rootfs_linux.go#L840-L842) in Docker.

# Docker Service

# Filesystem Layering

# Containers Summary

## Containers and Android

| Goal             | Containers                             | Android                          |
|------------------|----------------------------------------|----------------------------------|
| Namespace Mgmt.  | Separate *all* namespaces              | Intents for shared services      |
| App Isolation    | Namespace separation = isolation       | `uid` separation                 |
| Dependency Mgmt. | Container includes set of dependencies | App compiled for Android version |
