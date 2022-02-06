
## Exercises & Questions

### Library Trade-offs

- *Question:*
    What types of systems might want to always use only static libraries?
- *Question:*
    What types of systems might want to always use only dynamic libraries?
- *Question:*
    OSX uses dynamically linked libraries for a few common and frequently used libraries (like `libc`), and static libraries for the rest.
    Why?

### `chaosmalloc`

Netflix popularized testing their distributed infrastructure by intentionally injecting faults into the system.
This makes sense: most "error paths" in your code are often the least tested pieces of code because sometimes it is hard to cause the failures at the right time to test that code.
They created `chaosmonkey` -- a utility to add some chaos, and induce faults in the system to make sure the entire system doesn't go down.

Lets implement `chaosmalloc`!
How can you test that all of your error paths that check the return value of `malloc` are working correctly?
Cause random `malloc` failures, of course!

We don't want to modify malloc, so instead, we'll use `LD_PRELOAD` to ask the loader to interpose our `chaosmalloc` library on all calls to `malloc`.
Later, we'll use this to implement our own `malloc`/`free` in a homework.
