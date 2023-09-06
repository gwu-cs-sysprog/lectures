#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "domain_sockets.h"

#define MAX_BUF_SZ 1024

char filename_arg[256] = {'\0'} ;

/* Modified by Sibin on Sept. 06, 2023.
 * changing from on_exit() that is nonstandard to atexit()
 * function signature changed to match that of atexit()
 * 
 * Use the global arg for the filename to unlink
 */
// void onexit_remove_domain_socket(int status, void *filename)
void atexit_remove_domain_socket()
{
	if ( !strcmp(filename_arg, "") ) {
		unlink(filename_arg);
		// free(filename_arg);
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

	/* Modified by Sibin on Sept. 06, 2023.
	 * changing from on_exit() that is nonstandard to atexit()
	 * copy the filename to a global
	 * register the atexit() function
	 */
	// on_exit(onexit_remove_domain_socket, strdup(argv[1]));
	strcpy( filename_arg, argv[1] ) ;
	atexit( atexit_remove_domain_socket ) ;

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
