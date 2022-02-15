
## IPC Exercises

Find the programs:

- [`06/domain_socket_server.c`](https://github.com/gwu-cs-sysprog/lectures/blob/main/06/domain_socket_server.c) - a sample server.
- [`06/domain_socket_client.c`](https://github.com/gwu-cs-sysprog/lectures/blob/main/06/domain_socket_client.c) - a sample client.

Both require the [`06/domain_sockets.h`](https://github.com/gwu-cs-sysprog/lectures/blob/main/06/domain_sockets.h) header file.

Your job is to try to figure what in the hey these do!
Answer the following questions:

- How does the server handle each client request?
- How many clients can the server handle at a time?
- Is the number of clients limited by the server `wait`ing on client/child processes?
- Describe what the client does.
    What data does it send to the server?
- Describe the behavior when you use the client with the server.

The programs can be compiled directly with `gcc`.
Use these programs on the command line!

- How can you use the client and server on the command line?
