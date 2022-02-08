/*
 * Graciously modified from
 * https://github.com/troydhanson/network/tree/master/unixdomain
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_BUF_SZ 128

int
domain_socket_server_create(const char *file_name)
{
	struct sockaddr_un addr;
	int fd;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, file_name, sizeof(addr.sun_path)-1);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind error");
		exit(-1);
	}

	if (listen(fd, 5) == -1) {
		perror("listen error");
		exit(-1);
	}

	return fd;
}

char *socket_path;

void
remove_domain_socket(void)
{
	if (socket_path) unlink(socket_path);
}

int
main(void)
{
	char buf[MAX_BUF_SZ];
	int fd, new_client;
	int amnt;

	socket_path = getenv("DOMAIN_SOCKET_FILENAME");
	assert(socket_path);

	fd = domain_socket_server_create(socket_path);
	atexit(remove_domain_socket);

	/* We use this new descriptor to communicate with *this* client */
	if ((new_client = accept(fd, NULL, NULL)) == -1) {
		perror("accept error");
		exit(EXIT_FAILURE);
	}
	printf("Server: New client connected with new file descriptor %d.\n", new_client);

	amnt = read(new_client, buf, MAX_BUF_SZ - 1);
	if (amnt == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	buf[amnt] = '\0';
	printf("Server received request (sz %d): %s\n", amnt, buf);
	if (write(new_client, buf, amnt) < 0) {
		perror("write error in reply to client");
		exit(EXIT_FAILURE);
	}
	/* Done with communication with this client */
	close(new_client);

	return 0;
}
