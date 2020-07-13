
//#define _GNU_SOURCE
#include "file_access.h"
#include <ctype.h>
#include <stdio.h>

#include <queue>
#include <iostream>


std::queue<packet> fifoIn;
int tun_fd;
char tun_name[IFNAMSIZ];
uint8_t* pWr;
uint8_t bufWr[PACKET_BUF_SIZE];
uint8_t bufRd[PACKET_BUF_SIZE];

extern "C" {

// BEGIN Public interface
int file_access_init(int stop_until_1st_packet) {
/* Connect to the device */
  strcpy(tun_name, "tap2");
  tun_fd = tun_alloc(tun_name, IFF_TAP);  /* tun interface */

  if(tun_fd < 0){
    perror("Allocating interface");
    exit(1);
  }
  if(stop_until_1st_packet) {
  	//block until a packet is received
  	while(1) {
    	int p = fetch_packet(tun_fd, fifoIn);
			if(pending(fifoIn) >= 0) break;
			sleep(1000);
  } 
  }
  return tun_fd;
}

int file_access_write(int x) {

  return write_eth(pWr, x) ;
}

void file_access_flush() {
  flush(tun_fd, pWr, PACKET_BUF_SIZE);
}


int file_access_pending() {
  return pending(fifoIn);
}

int file_access_read() {
  return read_eth(fifoIn);
}

int file_access_fetch_packet() {
  return fetch_packet(tun_fd, fifoIn);
}  

// END Public Interface


int write_eth(uint8_t* pWr, int x) {
  bool ret = false;
  if (pWr < &bufWr[PACKET_BUF_SIZE]) { // buffer full?
    if ((x >= 0) && (x <= 65535)) {
      uint xHi = (uint8_t)(x >> 8), xLo = (uint8_t)x;
      *pWr++ = xHi; *pWr++ = xLo;
      ret = true; // word has 8b value 0-255 ?
    }  
  }
  return ret;
}



void flush(int tun_fd, uint8_t* pWr, size_t n) {
  ssize_t nwrite = write(tun_fd, bufWr, n);
  CHECK(n == nwrite);
  pWr = &bufWr[0];
}


int pending(std::queue<packet>& fifo) {
  int ret = 0;
  if      (fifo.empty())         ret = FIFO_EMPTY;
  else if (fifo.front().empty()) ret = PACKET_EMPTY;
  
  return ret;
}

int read_eth(std::queue<packet>& fifo) {
  int ret = FIFO_EMPTY;
  if (!fifo.empty()) 
  {
    if (!fifo.front().empty()) 
    { 
      ret = fifo.front().front() << 8;
      fifo.front().pop();
      // look for low word. if packet not empty, OR it into return value and pop again. If empty, do nothing (OR with 0)
      if (!fifo.front().empty()) { ret |= fifo.front().front(); fifo.front().pop(); }
    } else {
      ret = PACKET_EMPTY;
      fifo.pop();
    }
  } 
  return ret;
}


void enqueuePacket(std::queue<packet>& fifo, uint8_t *p, size_t n) {
  packet tmp;
  for(int i=0;i<n;i++) tmp.push((int)p[i]);
  fifo.push(tmp);
}

void hexdump(void *ptr, int buflen) {
  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=16) {
    printf("%06x: ", i);
    for (j=0; j<16; j++) 
      if (i+j < buflen)
        printf("%02x ", buf[i+j]);
      else
        printf("   ");
    printf(" ");
    for (j=0; j<16; j++) 
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
    printf("\n");
  }
}


void swap(uint8_t *p, uint8_t *q, int nbytes)
{
  for (int i = 0; i < nbytes; i++) {
    uint8_t t = *p; *p = *q; *q = t;
    p++; q++;
  }
}

bool doarp(uint8_t *p, size_t nbytes)
{
  (void)nbytes; 
  uint16_t op = ntohs(get16(p+14+6));
  char fromaddr[INET_ADDRSTRLEN];
  char toaddr[INET_ADDRSTRLEN];
  // Skip 14 bytes of ethernet header
  inet_ntop(AF_INET, p+14+14, fromaddr, sizeof(fromaddr));
  inet_ntop(AF_INET, p+14+24, toaddr, sizeof(toaddr));
  // Assume ethernet and IPv4
  printf("proto=ARP op=%u src=%s dst=%s\n",
         op, fromaddr, toaddr);
  // Now construct the ARP response
  put16(p+14+6,htons(2)); // Operation
  uint8_t *mac = p+14+18;
  mac[0] = 0x02; mac[1] = 0x00;
  memcpy(mac+2,p+14+24,4); // Use expected IP as top 4 bytes of MAC
  memcpy(p,mac,6); // Copy to source (it will be swapped later).
  swap(p+14+8,p+14+18,10);
  return true;
}

bool processtap(std::queue<packet>& fifo, uint8_t *p, size_t nbytes)
{
  uint16_t etype;
  memcpy(&etype,p+12,2);
  etype = ntohs(etype);
  printf("Frame etype=%04x nbytes=%zu\n", etype, nbytes);
  //hexdump(p, nbytes);
  bool respond = false;
  if (etype == 0x0806) {
    //handle ARP
    respond = doarp(p,nbytes);
    printf("Answering ARP\n");
  } else if (etype == 0x0800) {
    enqueuePacket(fifo, p, nbytes);
  }
  if (respond) swap(p,p+6,6);
  return respond;
}

int fetch_packet(int tun_fd, std::queue<packet>& fifo) {
  bool foundIncomingPacket = false;
  int nread;
 
      /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
  nread = read(tun_fd,bufRd, PACKET_BUF_SIZE);
  if(nread > 0) {
    /* Do whatever with the data */
    foundIncomingPacket = true;
    bool respond = processtap(fifo, bufRd, nread);
    if (respond) {
      ssize_t nwrite = write(tun_fd,bufRd, nread);
      CHECK(nwrite == nread);
    }
  }
  return foundIncomingPacket;
}

int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  const char *clonedev = "/dev/net/tun";

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR | O_NONBLOCK )) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
}

}