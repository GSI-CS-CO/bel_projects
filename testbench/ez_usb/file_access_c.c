#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>

#define VARNAME (work__file_access__my_var)
extern int VARNAME;

void say_hello(int n) {
  //printf("Hello %d %x\n", n, n);
  //fflush(stdout);
}

struct pollfd pfds[1];
unsigned char write_buffer[256] = {0,};
int write_buffer_length = 0;

void file_access_init(int pts) {
	char name_buffer[256];
	sprintf(name_buffer,"/dev/pts/%d", pts);
	pfds[0].fd = open(name_buffer, O_RDWR | O_DSYNC | O_NONBLOCK);
	pfds[0].events = POLLIN;
	printf("poll ...\n");
	poll(pfds,1,-1);
	printf("... done\n");
}

int file_access_read(int timeout) {
	unsigned char ch = 0;
	pfds[0].events = POLLIN;
	if (poll(pfds,1,timeout) == 0) {
		//printf("file_access_read timed out\n");
		return -1;
	}
	ssize_t result = read(pfds[0].fd, &ch, 1);
	if (result == 1) {
		printf("file_access_read %d %d\n", result, (int)ch);
		result = 0;
		return ch;
	} else if (result == -1) {
		printf("error while read %d %s\n", errno, strerror(errno));
	}
	return -1;
}

void file_access_write(int x) {
	unsigned char ch = x;
	write_buffer[write_buffer_length++] = ch;
	printf("file_access_write %x\n", (int)x);
}

void file_access_flush() {
	pfds[0].events = POLLOUT;
	poll(pfds,1,-1);
	int result = write(pfds[0].fd, write_buffer, write_buffer_length);
	write_buffer_length = 0;
	if (result == -1) {
		printf("error while write %d %s\n", errno, strerror(errno));
	}
	printf("all written %d\n", result);
}

