/*
 * Graciously taken from
 * https://github.com/troydhanson/network/tree/master/unixdomain
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define MAX_BUF_SZ 128

int
domain_socket_client_create(const char *file_name)
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

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	return fd;
}

int
main(void)
{
	char buf[] = "The most important message: penny for pawsident!\n";
	char msg[MAX_BUF_SZ];
	int fd;
	int amnt = 0;
	char *socket_path;

	socket_path = getenv("DOMAIN_SOCKET_FILENAME");
	assert(socket_path != NULL);
	fd = domain_socket_client_create(socket_path);

	printf("Client connected to server.\n");

	amnt = write(fd, buf, strlen(buf) + 1);
	if (amnt < 0) {
		perror("write error");
		exit(EXIT_FAILURE);
	}
	printf("Client request sent sent to server.\n");

	if (read(fd, msg, amnt) < 0) {
		perror("error during reading reply");
		exit(EXIT_FAILURE);
	}
	msg[amnt] = '\0';
	printf("Client reply received from server: %s\n", msg);
	close(fd);

	return 0;
}
