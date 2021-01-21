#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

#define VARNAME (work__ez_usb_dev__my_var)
#define TIMEOUT (work__ez_usb_dev__timeout)
#define HANGUP (work__ez_usb_dev__hangup)
extern int VARNAME;
extern int TIMEOUT;
extern int HANGUP;

struct pollfd pfds[1] = {0,};
unsigned char write_buffer[32768] = {0,};
int write_buffer_length = 0;

int record_read_count = 0;
int record_write_count = 0;


void ez_usb_dev_init(int stop_until_connected) {
	if (stop_until_connected && pfds[0].fd != 0) {
		close(pfds[0].fd);
		pfds[0].fd = 0;
	}
	if (pfds[0].fd == 0) {
		int fd = open("/dev/ptmx", O_RDWR | O_NONBLOCK);
		// print the name of the pseudo terminal in device tree
		char name[256];
		ptsname_r(fd, name, 256);
		printf("eb-device : %s\n",name);

		// put it in raw mode
	   struct termios raw;
		if (tcgetattr(fd, &raw) == 0)
		{
			// input modes - clear indicated ones giving: no break, no CR to NL, 
			//   no parity check, no strip char, no start/stop output (sic) control 
			raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

			// output modes - clear giving: no post processing such as NL to CR+NL 
			raw.c_oflag &= ~(OPOST);

			// control modes - set 8 bit chars 
			raw.c_cflag |= (CS8);

			// local modes - clear giving: echoing off, canonical off (no erase with 
			//   backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) 
			raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

			// control chars - set return condition: min number of bytes and timer 
			raw.c_cc[VMIN] = 5; raw.c_cc[VTIME] = 8; // after 5 bytes or .8 seconds
			                                         //   after first byte seen   
			raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0; // immediate - anything      
			raw.c_cc[VMIN] = 2; raw.c_cc[VTIME] = 0; // after two bytes, no timer 
			raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 8; // after a byte or .8 seconds

			// put terminal in raw mode after flushing 
			if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) 
			{
				int err = errno;
				printf("Error, cant set raw mode: %s\n", strerror(err));
				return;
			}
		}

		if (stop_until_connected) {
			printf("waiting for client, simulation stopped ...");
		} else {
			printf("device is ready, simulation is running\n");
		}
		fflush(stdout);
		grantpt(fd);
		unlockpt(fd);
		pfds[0].fd = fd;
	}
	if (stop_until_connected)
	{
		pfds[0].events = POLLIN;
		poll(pfds,1,-1);
		printf(" connected, simulation continues\n");
	}
}

int ez_usb_dev_read(int timeout_value) {
	record_write_count = 0;
	//printf("ez_usb_dev_read ");
	static int count = 0;
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
		if ((count%4)==0) {
			printf("%6d << ", record_read_count++);
		}
		printf("%02x", (int)ch);
		++count;
		if ((count%4)==0) {
			printf("\n");
		}
		return ch;
	} else if (result == -1) { // error
		printf("error while read %d %s\n", errno, strerror(errno));
	}
	return TIMEOUT;
}

void ez_usb_dev_write(int x) {
	record_read_count = 0;
	static int count = 0;
	unsigned char ch = x;
	write_buffer[write_buffer_length++] = ch;
	//printf("        >> %02x\n", (int)x);
	if ((count%4)==0) {
		printf("%6d  >> ", record_write_count++);
	}
	printf("%02x", (int)x);
	++count;
	if ((count%4)==0) {
		printf("\n");
	}
}

void ez_usb_dev_flush() {
	for (int i = 0; i < 79; ++i) printf("-");
	printf("\n");
	pfds[0].events = POLLOUT;
	poll(pfds,1,-1);
	int result = write(pfds[0].fd, write_buffer, write_buffer_length);
	write_buffer_length = 0;
	if (result == -1) {
		printf("error while write %d %s\n", errno, strerror(errno));
	}
	//printf("all written %d\n", result);
}

