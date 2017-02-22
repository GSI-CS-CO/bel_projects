//
// eb_memtest with pattern 0x5555555 and 0xAAAAAAAA on SCU wb-shared DDR3 
// Version 1.0 created: 22.02.2017, Karlheinz Kaiser
// Physical DDR1 I/F address found by eb_sdb_find

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

//Etherbone
#include <etherbone.h>



//vendor IDs
#define WB_CERN         0xce42
#define WB_GSI          0x0651


//device product ID from mini_sdb.h, Vendor from wb_vendors.h, this one is c_wb_DDR3_if1_sdb
#define WB_DDR3_USER_VENDOR      WB_GSI      //vendor ID
#define WB_DDR3_USER_PRODUCT     0x20150828  //product ID
#define WB_DDR3_USER_VMAJOR      1           //major revision
#define WB_DDR3_USER_VMINOR      0           //minor revision

//device product ID from mini_sdb.h, Vendor from wb_vendors.h, this one is c_wb_DDR3_if1_sdb
#define WB_FIFO_USER_VENDOR      WB_GSI      //vendor ID
#define WB_FIFO_USER_PRODUCT     0x20160525  //product ID
#define WB_FIFO_USER_VMAJOR      1           //major revision
#define WB_FIFO_USER_VMINOR      0           //minor revision


#define SDB_DEVICES   2


static const char* program;

void itoa(unsigned int n,char s[], int base){
     int i;
 
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % base + '0';   /* get next digit */
     } while ((n /= base) > 0);     /* delete it */
     s[i] = '\0';
}

// routines for message and error outputs

void die_eb(const char* where,eb_status_t status) {
  fprintf(stderr,"%s: %s failed: %s\n",
    program,where, eb_status(status));
  exit(1);
}

void die(const char* where,eb_status_t status) {
  fprintf(stderr,"%s: %s failed: %s\n",
    program,where, eb_status(status));
  exit(1);
}


// For interruption of output routine due to keyboard hit

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}


//--------------------------------------------------------------------------------------------

int main(int argc, const char** argv) {
  eb_status_t status;
  eb_device_t device;
  eb_socket_t socket;
  struct sdb_device sdbDevice[SDB_DEVICES];
  eb_cycle_t  cycle;
  eb_address_t sharedRAM;
  eb_address_t xfer_FIFO;

  eb_address_t lowaddr;
  eb_address_t highaddr;

  //eb_data_t   temp_data;

  const char* devName;
  int nDevices;
  int i; 
  int imax;
  int k;
  int l;
  int lmax;


  int writes_performed;
  int reads_performed;
  int read_errors;

  eb_data_t ddr_lw[256]; 
  eb_data_t ddr_hw[256];
  eb_data_t lowdata;
  eb_data_t highdata;

  
  program = argv[0];
  if (argc < 2) {
    fprintf(stderr,"Performs memtest on SCU DDR3   \n");
    fprintf(stderr,"Syntax: %s <protocol/host/port>\n", argv[0]);
    fprintf(stderr,"Test will need  about 20 min   \n");
    fprintf(stderr,"Progress is shown with \".\" for writes and \":\" for reads   \n");  
    fprintf(stderr,"Abort test with CTRL-C         \n");  
    fprintf(stderr,"Errors are reported on Screen  \n"); 
    return 1;
  }
  
  devName     = argv[1];
  
  /* Open a socket supporting only 32-bit operations.
   * As we are not exporting any slaves we don't care what port we get => 0.
   * This function always returns immediately.
   * EB_ABI_CODE helps detect if the application matches the library.
   */
  if ((status = eb_socket_open(EB_ABI_CODE,0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open",status);
  

  /* Open the remote device with 3 attempt is to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open see eb_device_open_nb.
   * Note: the supported widths can never be more than the socket supports.
   */
  if ((status = eb_device_open(socket,devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open",status);

  
    /* addr of DDR3 WB1 Interface */
    nDevices = SDB_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, WB_DDR3_USER_VENDOR, WB_DDR3_USER_PRODUCT, sdbDevice, &nDevices)) != EB_OK)
    die_eb("DDR3 eb_sdb_find_by_identity", status);
  if (nDevices == 0)
    die_eb("no DDR3 found in sdb", EB_FAIL);

  sharedRAM = sdbDevice[0].sdb_component.addr_first;
  printf("DDR3 sharedRAM found at address: 0x%"EB_DATA_FMT"\n", sharedRAM);



  /* addr of DDR3 WB2 Interface */
    nDevices = SDB_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, WB_FIFO_USER_VENDOR, WB_FIFO_USER_PRODUCT, sdbDevice, &nDevices)) != EB_OK)
    die_eb("DDR3 eb_sdb_find_by_identity", status);
  if (nDevices == 0)
    die_eb("no DDR3 found in sdb", EB_FAIL);
    
  xfer_FIFO = sdbDevice[0].sdb_component.addr_first;
  printf("DDR3 xfer_FIFO found at address: 0x%"EB_DATA_FMT"\n", xfer_FIFO);

  //-------------------------------------------------------------------------------------------------------------------------
  // do write of DDR3 in chunks of 100, otherwise etherbone runs in time_out

  k=1;
  imax=99;
  lmax=167771; //the real size 167771 * 100 = 16.777.100 loops(each loop two 32 bit writes) 
               //whole ram is 16777216 Words, each 64 bit, which is 1GBit for MT41J64M16LA-15E
               //116 words untested due to index granularity
  //lmax=2;

  for (k=1; k<=2; k++)
  {
    if (k==1) 
      {
         lowdata=0xaaaaaaaa;
        highdata=0xaaaaaaaa;
      }

    if (k==2) 
      {
         lowdata=0x55555555;
        highdata=0x55555555;
     }

    writes_performed=0;
    reads_performed=0;
    read_errors=0;

    i=0;
    l=0;



    for (l = 0 ; l <= lmax; l++) 
    {

      if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
        die("EP eb_cycle_open", status);
   
    for (i = 0; i <= imax; i++) 
    {

      lowaddr=sharedRAM  +8*i   + 100*l*8;
      highaddr=sharedRAM +8*i+4 + 100*l*8;

      //printf("WR l=%ld i=%ld  lwaddr=%lx  hwaddr=%lx \n", (unsigned long)l, (unsigned long)i, (unsigned long)lowaddr, (unsigned long)highaddr );

      eb_cycle_write(cycle, highaddr,     EB_BIG_ENDIAN|EB_DATA32, highdata);
      eb_cycle_write(cycle, lowaddr,      EB_BIG_ENDIAN|EB_DATA32, lowdata);
      writes_performed=writes_performed+2;
      //printf(".");
    }
 
    
    if ((status = eb_cycle_close(cycle)) != EB_OK)
      die("EP eb_cycle_close", status);

    }
    printf("\nStep %d :Write on %d DDR3 addresses done.\n", k, writes_performed);

    //-------------------------------------------------------------------------------------------------------------------------
    // do readout of DDR3 in chunks of 100, otherwise etherbone runs in time_out
    // due to granularity of 100 readout ist done to 2fffff7c instead of 2fffffec (last HW)

    i=0;
    l=0;


    for (l = 0; l <= lmax; l++) 
    {
      if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
        die("EP eb_cycle_open", status);
  
      for (i = 0; i <= imax; i++) 
      {

        lowaddr=sharedRAM  +8*i   + 100*l*8;
        highaddr=sharedRAM +8*i+4 + 100*l*8;

        //printf("RD l=%ld i=%ld  lwaddr=%lx  hwaddr=%lx \n", (unsigned long)l, (unsigned long)i, (unsigned long)lowaddr, (unsigned long)highaddr );

        eb_cycle_read(cycle, lowaddr,     EB_DATA32|EB_BIG_ENDIAN, &ddr_lw[i]);
        eb_cycle_read(cycle, highaddr,    EB_DATA32|EB_BIG_ENDIAN, &ddr_hw[i]);
        reads_performed=reads_performed+2;
      }
 
      if ((status = eb_cycle_close(cycle)) != EB_OK)
        die("EP eb_cycle_close", status);
  
      for (i = 0; i <= imax; i++) 
      {
        lowaddr=sharedRAM  +8*i   + 100*l*8;
        highaddr=sharedRAM +8*i+4 + 100*l*8;

        if (ddr_lw[i] != lowdata )
        {
          printf("LWaddr=%lx Expected 0x%lx and Read 0x%"EB_DATA_FMT"\n", (unsigned long)lowaddr,lowdata, ddr_lw[i]);
          read_errors=read_errors+1;
        }
        if (ddr_hw[i] != highdata )
        {
          printf("HWaddr=%lx Expected 0x%lx and Read 0x%"EB_DATA_FMT"\n", (unsigned long)lowaddr,highdata, ddr_lw[i]);
          read_errors=read_errors+1;
        }
        //printf(":");

      }

    }
    printf("\nStep %d :Reads on %d DDR3 addresses done.\n", k,  reads_performed);
    printf("####################Summary DDR3 RAMTEST Step %d on SCU####################",k);
    printf("\nDDR Datawords used for test 0x%lx 0x%lx \n", lowdata, highdata);
    printf("\nDDR Writes performed %d \n", writes_performed);
    printf("\nDDR Reads performed  %d \n", reads_performed);
    printf("\nDDR Read errors      %d \n", read_errors);
    printf("###########################################################################\n");
  }


  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close",status);

  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close",status);


  return 0;
 
}
