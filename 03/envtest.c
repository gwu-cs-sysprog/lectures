#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int
main(int argc, char *argv[])
{
	char *e = "NOARG";
	char *v = "NOVAL";

	if (argc == 2) {
		e = argv[1];
		v = getenv(e);
		if (!v) {
			v = "";
		}
	}

	printf("Environment variable %s -> %s\n", e, v);

	return 0;
}
