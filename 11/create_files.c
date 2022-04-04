#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int
main(int argc, char * argv[])
{
	int i, fd;

	for(i = 0; i < argc; i++){
		/* create an empty file */
		if((fd = open(argv[i],O_CREAT,0666) > 0) > 0){
			close(fd);
		} else {
			perror("open");
		}
	}

	return 0;
}
