#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
	char cmd[1024] = "/bin/cat ./"; /* will append to this string */
	char input[40];

	printf("What input do you want to 'cat' (choose from below)\n");
	system("/bin/ls"); /* show available files */

	printf("input > ");
	fflush(stdout); /* force stdout to print */

	scanf("%s",input);/* read input */
	/* create the command, appending the command to the cat command */
	strcat(cmd,input);

	printf("Executing: %s\n", cmd);
	fflush(stdout); /* force stdout to print */

	system(cmd);

	return 0;
}
