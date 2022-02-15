#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "domain_sockets.h"

#define MAX_BUF_SZ 1024

void
onexit_remove_domain_socket(int status, void *filename)
{
	if (filename) {
		unlink(filename);
		free(filename);
	}
}

int
main(int argc, char *argv[])
{
	int socket_desc;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <domain_socket_file_name>\n", argv[0]);
		return EXIT_FAILURE;
	}

	socket_desc = domain_socket_server_create(argv[1]);
	if (socket_desc < 0) {
		perror("server domain socket creation");
		return EXIT_FAILURE;
	}
	on_exit(onexit_remove_domain_socket, strdup(argv[1]));

	while (1) {
		int  new_client;

		/* We use this new descriptor to communicate with *this* client */
		if ((new_client = accept(socket_desc, NULL, NULL)) == -1) {
			perror("accept error");
			exit(EXIT_FAILURE);
		}
		if (fork() == 0) {
			char *args[] = {"cat", NULL};

			close(STDIN_FILENO);
			dup2(new_client, STDIN_FILENO);
			close(new_client);

			execvp(args[0], args);
		}
		close(new_client);

		waitpid(0, NULL, WNOHANG);
	}

	return 0;
}
