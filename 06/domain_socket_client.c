#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "domain_sockets.h"

#define MAX_BUF_SZ 128

int
main(int argc, char *argv[])
{
	int amnt = 0;
	int socket_desc;
	char chunkachunk[MAX_BUF_SZ];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <domain_socket_file_name>\n", argv[0]);
		return EXIT_FAILURE;
	}
	socket_desc = domain_socket_client_create(argv[1]);
	if (socket_desc < 0) {
		perror("client domain socket creation");
		return EXIT_FAILURE;
	}

	while (1) {
		if ((amnt = read(STDIN_FILENO, chunkachunk, MAX_BUF_SZ)) < 0) {
			perror("error during reading reply");
			exit(EXIT_FAILURE);
		}
		if (amnt == 0) exit(EXIT_SUCCESS);

		write(socket_desc, chunkachunk, amnt);
	}

	return 0;
}
