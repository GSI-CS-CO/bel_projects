//#define _GNU_SOURCE
#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>



#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/if_tun.h>

#define TIMEOUT -1

// Some handy macros to help with error checking
#define CHECKAUX(e,s)                            \
 ((e)? \
  (void)0: \
  (fprintf(stderr, "'%s' failed at %s:%d - %s\n", \
           s, __FILE__, __LINE__,strerror(errno)), \
   exit(0)))
#define CHECK(e) (CHECKAUX(e,#e))
#define CHECKSYS(e) (CHECKAUX((e)==0,#e))
#define CHECKFD(e) (CHECKAUX((e)>=0,#e))

#define STRING(e) #e

extern unsigned char write_buffer[32768];
extern int write_buffer_length;
extern unsigned char read_buffer[32768];
extern int read_buffer_length;

int file_access_init(char *dev, int devtype);

int file_access_read(int fd, int retries);

void file_access_write(int x);

void file_access_flush();

#endif