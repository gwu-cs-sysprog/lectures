# Exercise: Understanding Library Memory Consumption

## `libexample`

See the [`08/libexample/`](https://github.com/gwu-cs-sysprog/lectures/tree/main/08/libexample) directory for examples of using normal linking (`make naive`), static library linking (`make static`), and dynamic library linking (`make dynamic`).
The source includes:

- `prog` and `prog2` that are programs that use libraries.
- `example.c` is a single file that creates the first library which generates `libexample.a` and `libexample.so`.
- `example2.c` and `example2b.c` are the files that create the second library which generates `libexample2.a` and `libexample2.so`.

Each of the library objects uses around *750KB* of memory.
To help you set the `LD_LIBRARY_PATH` to execute the dynamic libraries, you can run the dynamic library examples using

```
$ ./run_dyn.sh ./prog_dynamic
```

1. Use `nm` and `ldd` to understand which libraries and object files within those libraries each of the programs require.
2. Use `ls -lh` to see the size of each of the resulting executable programs.
    Explain the differences in sizes.

## Library Trade-offs

- *Question:*
    What types of systems might want to always use only static libraries?
- *Question:*
    What types of systems might want to always use only dynamic libraries?
- *Question:*
    OSX uses dynamically linked libraries for a few common and frequently used libraries (like `libc`), and static libraries for the rest.
    Why?
