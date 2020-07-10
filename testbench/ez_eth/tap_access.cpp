
//#define _GNU_SOURCE
#include "file_access.h"
#include <ctype.h>
#include <stdio.h>

#include <queue>
#include <iostream>


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
#define FIFO_EMPTY -255

typedef std::queue<int> packet;




int file_access_write(uint8_t* pWr, uint8_t* bufWr, int w) {
  bool ret = false;
  if (pWr < &bufWr[PACKET_BUF_SIZE]) { // buffer full?
    if ((w >= 0) && (w <= 255)) {ret = true; *pWr++ = (uint8_t)w;} // word has 8b value 0-255 ?
  }
  return ret;
}

void file_access_flush(int tun_fd, uint8_t* pWr, uint8_t* bufWr, size_t n) {
  ssize_t nwrite = write(tun_fd, bufWr, n);
  CHECK(n == nwrite);
  pWr = &bufWr[0];
}


int file_access_pending(std::queue<packet>& fifo) {
  if (fifo.empty()) return FIFO_EMPTY;
  else return 0;
}

int file_access_read(std::queue<packet>& fifo) {
  int ret = FIFO_EMPTY;
  if (!fifo.empty()) 
  {
    if (!fifo.front().empty()) 
    { 
      ret = fifo.front().front();
      fifo.front().pop(); 
    } else {
      ret = PACKET_EMPTY;
      fifo.pop();
    }
  } 
  return ret;
}


void enqueuePacket(std::queue<packet>& fifo, uint8_t* buf, size_t n) {
  packet tmp;
  for(int i=0;i<n;i++) tmp.push((int)buf[i]);
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

int file_access_fetch_packet(int tun_fd, std::queue<packet>& fifo) {
  bool foundIncomingPacket = false;
  int nread;
  unsigned char buffer[PACKET_BUF_SIZE] = {0,};
      /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
  nread = read(tun_fd,buffer,sizeof(buffer));
  if(nread > 0) {
    /* Do whatever with the data */
    foundIncomingPacket = true;
    bool respond = processtap(fifo, buffer, nread);
    if (respond) {
      ssize_t nwrite = write(tun_fd,buffer,nread);
      CHECK(nwrite == nread);
    }
  }
  return foundIncomingPacket;
}

int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

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

int main(int argc, char *argv[])
{
  
/*
  char *progname = argv[0];
  char *devname = NULL;
  const char *usage = "Usage: %s [--v] [--tap] <prefix> [<devname>]\n";
  int devtype = IFF_TUN;
  int verbosity = 0;

  argc--; argv++;
  while (argc > 0 && argv[0][0] == '-') {
    if (strcmp(argv[0],"--v") == 0) {
      verbosity++;
    } else if (strcmp(argv[0],"--tap") == 0) {
      devtype = IFF_TAP;
    } else {
      fprintf(stderr, usage, progname);
      exit(0);
    }
    argc--; argv++;
  }
  if (argc > 1) {
      fprintf(stderr, usage, progname);
      exit(0);
  }
  if (argc > 0) {printf("argc > 0, %s\n", argv[0]); devname = argv[0];}

  
  */
 
  char tun_name[IFNAMSIZ];
  
  
  /* Connect to the device */
  strcpy(tun_name, "tap2");
  int tun_fd = tun_alloc(tun_name, IFF_TAP);  /* tun interface */

  if(tun_fd < 0){
    perror("Allocating interface");
    exit(1);
  }

  std::queue<packet> fifoIn;
  int wrBuf[PACKET_BUF_SIZE] = {0,};

  /* Now read data coming from the kernel */
  while(1) {
    int p = fetchPacket(tun_fd, fifoIn);

    int w = readWord(fifoIn);
    if(w >= 0) {std::cout << '\t' << w; }
    else if(w == PACKET_EMPTY) {std::cout << std::endl;}
  }  

}  