#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


#define VARNAME (work__file_access__my_var)
extern int VARNAME;

void say_hello(int n) {
  printf("Hello");
}

int fd_read, fd_write;

void file_access_init() {
	fd_read  = open("/dev/pts/42", O_RDONLY);
	fd_write = open("/dev/pts/44", O_WRONLY);
}

int file_access_read() {
	char ch;
	int result = read(fd_read, &ch, 1);
	if (result == 1) {
		return (int)ch;
	}
	return -1;
}

void file_access_write(int x) {
	char ch = (char)x;
	write(fd_write, &ch, 1);
}