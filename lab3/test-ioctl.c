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
	char foo[4] = "foo", bar[4] = "bar", buf[4] = "buf";

	n = open("test/nwrites_to_crash", O_RDONLY | O_CREAT, 0644);
	ioctl(n, IOCTL_SET, 4);

	a = open("test/a.txt", O_RDONLY | O_CREAT, 0644);
	close(a);

	ioctl(n, IOCTL_SET, -1);  // uncrash

	a = open("test/a.txt", O_WRONLY, 0644);
	write(a, bar, 3);
	close(a);
	
	a = open("test/a.txt", O_RDONLY, 0644);
	read(a, buf, 3);
	printf("a.txt before writing to b.txt: \"%s\"\n", buf);
	close(a);

	b = open("test/b.txt", O_WRONLY | O_CREAT, 0644);
	write(b, foo, 3);
	close(b);

	a = open("test/a.txt", O_RDONLY, 0644);
	read(a, buf, 3);
	printf("a.txt after writing to b.txt: \"%s\"\n", buf);
	close(a);
}
