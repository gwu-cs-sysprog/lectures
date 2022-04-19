#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
	uid_t uid,euid;
	gid_t gid,egid;
	uid_t saved_euid;

	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	saved_euid=euid;
	setuid(uid);
	printf("---- setuid(%d) ----\n",uid);

	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	setuid(saved_euid);
	printf("---- setuid(%d) ----\n",saved_euid);
	uid = getuid();
	gid = getgid();
	printf(" uid=%d  gid=%d\n",  uid,  gid);

	euid = geteuid();
	egid = getegid();
	printf("euid=%d egid=%d\n", euid, egid);

	return 0;
}
