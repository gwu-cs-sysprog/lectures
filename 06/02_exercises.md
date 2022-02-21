
## IPC Exercises

Find the programs:

- [`06/domain_socket_server.c`](https://github.com/gwu-cs-sysprog/lectures/blob/main/06/domain_socket_server.c) - a sample server.
- [`06/domain_socket_client.c`](https://github.com/gwu-cs-sysprog/lectures/blob/main/06/domain_socket_client.c) - a sample client.

Both require the [`06/domain_sockets.h`](https://github.com/gwu-cs-sysprog/lectures/blob/main/06/domain_sockets.h) header file.
You *must* run the server first to create the domain socket "file".
If you run the server, and it complains that "server domain socket creation", then you might need to `rm` the domain socket file on the command line first.
It already exists from a previous run of the server, so the server cannot create it again!

Your job is to try to figure what in the hey these do!
Answer the following questions:

- Describe what the server does to handle each client request.
- How many clients can the server handle at a time?
- Is the number of clients limited by the server `wait`ing on client/child processes?
    Recall when talking about background tasks in a shell that the `wait` is key to the system's concurrency/behavior.
- Describe what the client does.
    What data does it send to the server?
- Describe the behavior when you use the client with the server.

The programs can be compiled directly with `gcc` (e.g. `gcc domain_socket_server.c -o server; gcc domain_socket_client.c -o client`).
Use these programs on the command line to send data from the client to server.

- How can you use the client and server programs to send data between them?
- How can you make multiple clients connect to the server?
