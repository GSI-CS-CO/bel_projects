//
// eb_ddr: demo for write and read SCU wishbone-shared DDR3 
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

//#################################################################################################

int main(int argc, const char** argv) {
  eb_status_t status;
  eb_device_t device;
  eb_socket_t socket;
  struct sdb_device sdbDevice[SDB_DEVICES];
  eb_cycle_t  cycle;
  eb_address_t sharedRAM;
  eb_address_t xfer_FIFO;
  //eb_data_t   temp_data;

  const char* devName;
  int nDevices;
  int i; 
  eb_data_t ddr_d[18];
  eb_data_t read_data;

  
   program = argv[0];
  if (argc < 2) {
    fprintf(stderr,"Performs read and write on SCU DDR3 \n");
    fprintf(stderr,"Syntax: %s <protocol/host/port>\n", argv[0]);
    fprintf(stderr,"DDR3 Transparent Mode and DDR3 Fifo Batch read is checked \n");
    fprintf(stderr,"For Transparent Mode data at begin and end of DDR address range is written\n");


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

//#################################################################################################
  // during a open cycle do not write and read on same address !!
  // readouts are readable AFTER closing an open cycle (even for printf stuff!)
  // therefore buffers like ddr_d[] are needed for temporarely storage of results.

  if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
    die("EP eb_cycle_open", status);
  
  // do some DDR writes on increasing addresses, Preload HW and move both LW and HW to DDR

    eb_cycle_write(cycle, sharedRAM +0x4,      EB_BIG_ENDIAN|EB_DATA32, 0xadd00004);
    eb_cycle_write(cycle, sharedRAM,           EB_BIG_ENDIAN|EB_DATA32, 0xadd00000);
    
    eb_cycle_write(cycle, sharedRAM +0xc,      EB_BIG_ENDIAN|EB_DATA32, 0xadd0000c);
    eb_cycle_write(cycle, sharedRAM +0x8,      EB_BIG_ENDIAN|EB_DATA32, 0xadd00008);
    
    eb_cycle_write(cycle, sharedRAM +0x14,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00014);
    eb_cycle_write(cycle, sharedRAM +0x10,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00010);
    
    eb_cycle_write(cycle, sharedRAM +0x1c,     EB_BIG_ENDIAN|EB_DATA32, 0xadd0001c);
    eb_cycle_write(cycle, sharedRAM +0x18,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00018);

    eb_cycle_write(cycle, sharedRAM +0x24,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00024);
    eb_cycle_write(cycle, sharedRAM +0x20,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00020);
    
    eb_cycle_write(cycle, sharedRAM +0x2c,     EB_BIG_ENDIAN|EB_DATA32, 0xadd0002c);
    eb_cycle_write(cycle, sharedRAM +0x28,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00028);
    
    eb_cycle_write(cycle, sharedRAM +0x34,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00034);
    eb_cycle_write(cycle, sharedRAM +0x30,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00030);
    
    eb_cycle_write(cycle, sharedRAM +0x3c,     EB_BIG_ENDIAN|EB_DATA32, 0xadd0003c);
    eb_cycle_write(cycle, sharedRAM +0x38,     EB_BIG_ENDIAN|EB_DATA32, 0xadd00038);

  // write  last valid DDR3 address too

    eb_cycle_write(cycle, sharedRAM +0x7ffffec,     EB_BIG_ENDIAN|EB_DATA32, 0xa7ffffec);
    eb_cycle_write(cycle, sharedRAM +0x7ffffe8,     EB_BIG_ENDIAN|EB_DATA32, 0xa7ffffe8);
  
    if ((status = eb_cycle_close(cycle)) != EB_OK)
      die("EP eb_cycle_close", status);

    printf("\nWrite on 18 DDR3 addresses done.\n");

//#################################################################################################

    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
      die("EP eb_cycle_open", status);
  
  // do some reads on increasing DDR3 addresses
    eb_cycle_read(cycle, sharedRAM +0x0,  EB_DATA32|EB_BIG_ENDIAN, &ddr_d[0]);
    eb_cycle_read(cycle, sharedRAM +0x4,  EB_DATA32|EB_BIG_ENDIAN, &ddr_d[1]);
    eb_cycle_read(cycle, sharedRAM +0x8,  EB_DATA32|EB_BIG_ENDIAN, &ddr_d[2]);
    eb_cycle_read(cycle, sharedRAM +0xc,  EB_DATA32|EB_BIG_ENDIAN, &ddr_d[3]);
    eb_cycle_read(cycle, sharedRAM +0x10, EB_DATA32|EB_BIG_ENDIAN, &ddr_d[4]);
    eb_cycle_read(cycle, sharedRAM +0x14, EB_DATA32|EB_BIG_ENDIAN, &ddr_d[5]);
    eb_cycle_read(cycle, sharedRAM +0x18, EB_DATA32|EB_BIG_ENDIAN, &ddr_d[6]);
    eb_cycle_read(cycle, sharedRAM +0x1c, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[7]);
    eb_cycle_read(cycle, sharedRAM +0x20, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[8]);
    eb_cycle_read(cycle, sharedRAM +0x24, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[9]);
    eb_cycle_read(cycle, sharedRAM +0x28, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[10]);
    eb_cycle_read(cycle, sharedRAM +0x2c, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[11]);
    eb_cycle_read(cycle, sharedRAM +0x30, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[12]);
    eb_cycle_read(cycle, sharedRAM +0x34, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[13]);
    eb_cycle_read(cycle, sharedRAM +0x38, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[14]);
    eb_cycle_read(cycle, sharedRAM +0x3c, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[15]);
    eb_cycle_read(cycle, sharedRAM +0x7ffffe8, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[16]);
    eb_cycle_read(cycle, sharedRAM +0x7ffffec, EB_BIG_ENDIAN|EB_DATA32, &ddr_d[17]);

    if ((status = eb_cycle_close(cycle)) != EB_OK)
      die("EP eb_cycle_close", status);

    i=0;
    for (i = 0; i <= 17; i++) {
      printf("\n%04"EB_DATA_FMT"", ddr_d[i]);
    }
 
    printf("\nRead on 18 addresses done.\n");
 
//----------------------------------------------------------------------------------------------------------------------

    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
       die("EP eb_cycle_open", status);
       eb_cycle_write(cycle, sharedRAM +0x7fffff4,     EB_BIG_ENDIAN|EB_DATA32, 0x00000000);
    if ((status = eb_cycle_close(cycle)) != EB_OK)
       die("EP eb_cycle_close", status);
        
    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
       die("EP eb_cycle_open", status);
       eb_cycle_read  (cycle, sharedRAM +0x7fffff4,     EB_DATA32|EB_BIG_ENDIAN, &read_data);
    if ((status = eb_cycle_close(cycle)) != EB_OK)
       die("EP eb_cycle_close", status);

    printf("\nBurst_Startadr is set to: %04"EB_DATA_FMT"", read_data);

//----------------------------------------------------------------------------------------------------------------------

    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
      die("EP eb_cycle_open", status);
    eb_cycle_write(cycle, sharedRAM +0x7fffff8,     EB_BIG_ENDIAN|EB_DATA32, 0x00000006);
    if ((status = eb_cycle_close(cycle)) != EB_OK)
      die("EP eb_cycle_close", status);
    printf("\nXfer counter now set to :    6");

//----------------------------------------------------------------------------------------------------------------------

    
    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
       die("EP eb_cycle_open", status);
    eb_cycle_read  (cycle, xfer_FIFO +0x38,     EB_DATA32|EB_BIG_ENDIAN, &read_data);

    if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("EP eb_cycle_close", status);

    printf("\nXFER_FIFO Status is     :%04"EB_DATA_FMT"", read_data);
    printf("\n");
//----------------------------------------------------------------------------------------------------------------------
    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
    die("EP eb_cycle_open", status);

    eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[0]);
    eb_cycle_read  (cycle, xfer_FIFO +0x34,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[1]);
    eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[2]);
    eb_cycle_read  (cycle, xfer_FIFO +0x34,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[3]);
    eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[4]);
    eb_cycle_read  (cycle, xfer_FIFO +0x34,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[5]);
    eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[6]);
    eb_cycle_read  (cycle, xfer_FIFO +0x34,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[7]);
    eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[8]);
    eb_cycle_read  (cycle, xfer_FIFO +0x34,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[9]);
    eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[10]);
    eb_cycle_read  (cycle, xfer_FIFO +0x34,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[11]);



    if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("EP eb_cycle_close", status);
    

    printf("\nPrint FIFO contents LW, HW, LW, ... \n");
    for (i = 0; i <= 11; i++) {
      printf("\n%04"EB_DATA_FMT"", ddr_d[i]);
    }

    printf("\nFIFO is now empty \n");
    
//----------------------------------------------------------------------------------------------------------------------
    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
       die("EP eb_cycle_open", status);
    eb_cycle_read  (cycle, xfer_FIFO +0x38,     EB_DATA32|EB_BIG_ENDIAN, &read_data);

    if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("EP eb_cycle_close", status);

    printf("\nXFER_FIFO Status is     :%04"EB_DATA_FMT"", read_data);
    printf("\n");
//----------------------------------------------------------------------------------------------------------------------

    printf("\nReadout of FIFO which is empty causes cycle error \n");
    printf("\nNow checking for this cycle close error by performing an additional read \n");    
    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
    die("EP eb_cycle_open", status);

       eb_cycle_read  (cycle, xfer_FIFO +0x30,     EB_DATA32|EB_BIG_ENDIAN, &ddr_d[12]);

    if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("EP eb_cycle_close", status);

    //-- program is left here because eb_cycle_close failed
//----------------------------------------------------------------------------------------------------------------------

    /* close handler cleanly */
    if ((status = eb_device_close(device)) != EB_OK)
      die("eb_device_close",status);

    if ((status = eb_socket_close(socket)) != EB_OK)
      die("eb_socket_close",status);


  return 0;
 
}
