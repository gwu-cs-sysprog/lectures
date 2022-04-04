#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	/*
	 * Execute `cat`. It inherits the stdin/stdout/stderr
	 * of the current process.
	 */
	system("cat");

	return 0;
}
