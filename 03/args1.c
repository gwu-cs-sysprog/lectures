#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

char *prog = "./03/args2.bin";

int
main(int argc, char *argv[])
{
	int i;
	char **args; 		/* an array of strings */

	printf("Inside %s\n", argv[0]);

	/* lets just pass the arguments on through to args2! */
	args = calloc(argc + 1, sizeof(char *));
	assert(args);

	args[0] = prog;
	for (i = 1; i < argc; i++) {
		args[i] = argv[i];
	}
	args[i] = NULL; /* the arguments need to be `NULL`-terminated */

	if (execvp(prog, args)) {
		perror("exec");

		return EXIT_FAILURE;
	}

	return 0;
}
