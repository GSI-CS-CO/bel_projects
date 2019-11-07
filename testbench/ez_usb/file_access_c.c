#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define VARNAME (work__file_access__my_var)
#define TIMEOUT (work__file_access__timeout)
#define HANGUP (work__file_access__hangup)
extern int VARNAME;
extern int TIMEOUT;
extern int HANGUP;

struct pollfd pfds[1] = {0,};
unsigned char write_buffer[32768] = {0,};
int write_buffer_length = 0;


void file_access_init() {
	if (pfds[0].fd == 0) {
		int fd = open("/dev/ptmx", O_RDWR | O_NONBLOCK);
		// print the name of the pseudo terminal in device tree
		char name[256];
		ptsname_r(fd, name, 256);
		printf("eb-device : %s\n",name);
		printf("waiting for client ...");
		fflush(stdout);
		grantpt(fd);
		unlockpt(fd);
		pfds[0].fd = fd;
		pfds[0].events = POLLIN;
		poll(pfds,1,-1);
		printf(" connected\n");
	}
}

int file_access_read(int timeout_value) {
	//printf("file_access_read ");
	unsigned char ch = 0;
	pfds[0].events = POLLIN | POLLHUP;
	if (poll(pfds,1,timeout_value) == 0) {
		//printf("timeout\n");
		return TIMEOUT;
	}
	if (pfds[0].revents == POLLHUP) { // client disconnected
		//printf("hangup\n");
		return HANGUP;  
	}
	ssize_t result = read(pfds[0].fd, &ch, 1);
	if (result == 1) { // successful read
		//printf("%d\n", (int)ch);
		return ch;
	} else if (result == -1) { // error
		printf("error while read %d %s\n", errno, strerror(errno));
	}
	return TIMEOUT;
}

void file_access_write(int x) {
	unsigned char ch = x;
	write_buffer[write_buffer_length++] = ch;
	//printf("file_access_write %x\n", (int)x);
}

void file_access_flush() {
	pfds[0].events = POLLOUT;
	poll(pfds,1,-1);
	int result = write(pfds[0].fd, write_buffer, write_buffer_length);
	write_buffer_length = 0;
	if (result == -1) {
		printf("error while write %d %s\n", errno, strerror(errno));
	}
	//printf("all written %d\n", result);
}

