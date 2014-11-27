#include "ioctl.h"

#include <linux/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) {

	int fd = 0, r = 0, e = 0;

	fd = open("nwrites_to_crash", O_RDWR | O_CREAT, NULL);
	r = ioctl(fd, IOCTL_CRASH, NULL);
	e = errno;

	printf("fd %d\n", fd);
	printf("r %d\n", r);
	printf("e %d\n", e);
}
