#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
main(void)
{
	char cmd[1024] = "/bin/cat ./"; /* will append to this string */
	char input[40];
	int i;

	printf("What input do you want to 'cat' (choose from below)\n");
	system("/bin/ls"); /* show available files */

	printf("input > ");
	fflush(stdout); /* force stdout to print */

	scanf("%s",input);/* read input */

	/* clean input before passing to /bin/cat */
	for (i = 0; i < 40; i++) {
		if (input[i] == ';' || input[i] == '|' || input[i] == '$' || input[i] == '&') {
			input[i] = '\0'; /* change all ;,|,$,& to a NULL */
		}
	}

	/* concatenate the two strings */
	strcat(cmd,input);
	printf("Executing: %s\n", cmd);
	fflush(stdout);

	system(cmd);

	return 0;
}
