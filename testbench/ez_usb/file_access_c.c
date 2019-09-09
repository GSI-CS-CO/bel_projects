#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>


#define VARNAME (work__file_access__my_var)
extern int VARNAME;

void say_hello(int n) {
  //printf("Hello %d %x\n", n, n);
  //fflush(stdout);
}

int fd_read, fd_write;

void file_access_init() {
	fd_read  = open("/dev/pts/35", O_RDONLY | O_NONBLOCK);
	fd_write = open("/dev/pts/39", O_WRONLY);
	struct pollfd pfds[1];
	pfds[0].fd = fd_read;
	pfds[0].events = POLLIN;
	printf("poll ...\n");
	poll(pfds,1,10000);
	printf("... done\n");
}

void file_access_wait_for_input() {

}

int file_access_read() {
	unsigned char ch = 0;
	ssize_t result = read(fd_read, &ch, 1);
	printf("file_access_read %d %d\n", result, (int)ch);
	if (result == 1) {
		result = 0;
		return result+ch;
	}
	return -1;
}

void file_access_write(int x) {
	unsigned char ch = (unsigned char)x;
	int result = write(fd_write, &ch, 1);
	printf("file_access_write %d %x\n", result, (int)x);
}