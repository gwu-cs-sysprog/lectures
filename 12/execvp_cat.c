#include <unistd.h>

int
main(void)
{
	char * args[] = { "cat", NULL };

	execvp(args[0], args);

	return 0;
}
