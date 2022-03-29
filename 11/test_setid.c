#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	uid_t uid,euid;
	gid_t gid,egid;

	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	return 0;
}
