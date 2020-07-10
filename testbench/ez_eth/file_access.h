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

#define SRC_OFFSET4 12
#define DST_OFFSET4 16
#define SRC_OFFSET6 8
#define DST_OFFSET6 24
#define HLEN_OFFSET 0
#define PROTO_OFFSET 9
#define PROTO_ICMP 1
#define PROTO_IGMP 2
#define PROTO_TCP 6
#define PROTO_UDP 17

#define PACKET_BUF_SIZE 6000
#define PACKET_EMPTY -1
#define FIFO_EMPTY -2

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

typedef std::queue<int> packet;
std::queue<packet> fifoIn;
int tun_fd;
char tun_name[IFNAMSIZ];
uint8_t write_buffer[PACKET_BUF_SIZE];
uint8_t read_buffer[PACKET_BUF_SIZE];





// BEGIN Public interface

int file_access_init(int stop_until_1st_packet);
int file_access_write();
void file_access_flush();
int file_access_pending();
int file_access_read();
int file_access_fetch_packet();

// END Public Interface


int write(uint8_t* pWr, uint8_t* bufWr, int w);



void flush(int tun_fd, uint8_t* pWr, uint8_t* bufWr, size_t n);


int pending(std::queue<packet>& fifo);


void enqueuePacket(std::queue<packet>& fifo, uint8_t* buf, size_t n);

static inline void put16(uint8_t *p, uint16_t n)
{
  memcpy(p,&n,sizeof(n));
}

static inline uint16_t get16(uint8_t *p)
{
  uint16_t n;
  memcpy(&n,p,sizeof(n));
  return n;
}


void swap(uint8_t *p, uint8_t *q, int nbytes);

bool doarp(uint8_t *p, size_t nbytes);

bool processtap(std::queue<packet>& fifo, uint8_t *p, size_t nbytes);

int fetch_packet(int tun_fd, std::queue<packet>& fifo);

int tun_alloc(char *dev, int flags);


#endif