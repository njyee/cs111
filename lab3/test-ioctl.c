#include "ioctl.h"

#include <linux/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h> // malloc
#include <stdio.h>

int main(int argc, char *argv[]) {

	int a = -1, b = -1, n = -1;
	char foo[4] = "foo", buf[4] = "";

	n = open("test/nwrites_to_crash", O_RDONLY | O_CREAT, 0);
	ioctl(n, IOCTL_SET, 2);
	//creat("test/a.txt", 0);  // crash
	a = open("test/a.txt", O_RDWR | O_CREAT, 0);

	ioctl(n, IOCTL_SET, -1);  // uncrash
	//creat("test/b.txt", 0);
	b = open("test/b.txt", O_RDWR | O_CREAT, 0);

	//a = open("test/a.txt", O_RDWR | O_CREAT, 0);
	//b = open("test/b.txt", O_RDWR | O_CREAT, 0);

	read(a, buf, 4);
	printf("a.txt before writing to b.txt: \"%s\"\n", buf);

	write(b, foo, 4);

	read(a, buf, 4);
	printf("a.txt after writing to b.txt: \"%s\"\n", buf);
	//read(b, buf, 4);
	//printf("b.txt: \"%s\"\n", buf);



	// int fd = 0, r = 0, e = 0;
	// int a = -99, b = -99, c = -99, d = -99;
	// //char *s;
	// //s = (char *) malloc(20*sizeof(char));

	// fd = open("test/nwrites_to_crash", O_RDONLY, NULL);
	// // r = ioctl(fd, IOCTL_CRASH, 56);
	// // //r = read(fd, s, 3);
	// // e = errno;

	// // printf("fd %d\n", fd);
	// // printf("r %d\n", r);
	// // printf("e %d\n", e);

	// a = ioctl(fd, IOCTL_SET, -2);
	// // b = ioctl(fd, IOCTL_SET, -1);
	// // c = ioctl(fd, IOCTL_GET, 0);
	// // d = ioctl(fd, IOCTL_SET, 1);
	// e = errno;
	// printf("a %d\n", a);
	// // printf("b %d\n", b);
	// // printf("c %d\n", c);
	// // printf("d %d\n", d);
	// printf("e %d\n", e);
}
