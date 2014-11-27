#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define MAJOR_NUM 100
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)

int main()
{
	int r = 0;
	int fd = open("var", O_CREAT | O_RDWR | O_NONBLOCK);
	r = ioctl(fd, 0, "test");
	//r = errno;
	//write(fd, '\0', 2);
	close(fd);
	if (r) {
		printf("error %d ", r);
	}
}
