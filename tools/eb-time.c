#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <etherbone.h>
#include <time.h>

#define SNTPRINTF(b, ...) ((b) += sprintf((b), __VA_ARGS__))
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define MAX_DEVICES 1
#define PPS_CNTR_UTCLO  0x8
#define PPS_CNTR_UTCHI  0xC
#define PPS_STATE       0x1C
#define PPS_VALID       0x2
#define TS_VALID        0x4


eb_device_t device0;
eb_device_t device1;

const char* program;

static int die(eb_status_t status, const char* what)
{  
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  return -1;
}




static void help(void) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> [etherbone-device] \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                        display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  Shows WhiteRabbit time and status.\n");
  fprintf(stderr, "  When two devices are supplied, UTC times are compared as well.\n");
  fprintf(stderr, "\n");

  //fprintf(stderr, "  condget                   debug feature. if this core is wating for a condition value, ask what it is\n");
  //fprintf(stderr, "  condset                   debug feature. set condition value in shared mem for this core\n");
  fprintf(stderr, "\n");
}

int show(uint64_t wrnow, uint32_t wr_status, const char* netaddress) {
  char strBuff[65536];
  char* pSB = (char*)&strBuff;
  char sLinkState[20];
  char* pL = (char*)&sLinkState;
  char sSyncState[20];
  char* pS = (char*)&sSyncState;
  

  //Generate WR Status
  
  //55d32f8f
  
  if(wr_status & PPS_VALID) SNTPRINTF(pL ,"  %sOK%s  ", KGRN, KNRM);
  else                      SNTPRINTF(pL ,"  %s--%s  ", KRED, KNRM);
  if(wr_status & TS_VALID)  {
    //check if it's at least 1980something
    if (wrnow & 0x40000000ull) SNTPRINTF(pS ,"  %sOK%s  ", KGRN, KNRM); 
    else                       SNTPRINTF(pS ,"  %s<<%s  ", KYEL, KNRM);
  }  
  else                         SNTPRINTF(pS ,"  %s--%s  ", KRED, KNRM);
  
  time_t wrtime = (time_t)wrnow;
  SNTPRINTF(pSB , "%s%30s%s    PPS: %s TS: %s WR-UTC: %.24s\n", KCYN, netaddress, KNRM, sLinkState, sSyncState, ctime((time_t*)&wrtime));

  printf("%s", (const char*)strBuff);

  return 0;
}

int main(int argc, char** argv) {

   //vars for command line args
   program  = argv[0];
   
   int opt;
   const char *netaddress0, *netaddress1;

   
   uint32_t wr_utc_hi[2],
            wr_utc_lo[2],
            wr_status[2];
   uint64_t wr_now[2];         
   
   int error = 0;
   int compare = 0;
   
   // start getopt 
   while ((opt = getopt(argc, argv, "c:h")) != -1) {
      switch (opt) {
         case 'h':
            help();
            return 0;
         case ':':
         case '?':
            break;
         default:
            fprintf(stderr, "%s: bad getopt result\n", program);
            return 1;
      }
   }

   if (error) return 1;

   if (optind >= argc) {
   fprintf(stderr, "%s: expecting one or two arguments: <etherbone-device> <etherbone-device>\n", program);
   fprintf(stderr, "\n");
   help();
   return 1;
   }
   
   
   
   netaddress0 = argv[optind];
   if (optind+1 < argc) {
    netaddress1 = argv[optind+1];
    compare = 1;
   } 
   printf("\n");

  eb_socket_t socket0, socket1;
  eb_device_t device0, device1;
  eb_status_t status;
  eb_cycle_t cycle;
  

   eb_address_t ppsAdr0;
   eb_address_t ppsAdr1;
   eb_data_t tmpRead[6];
  int attempts;
  int num_devices;
  struct sdb_device devices[MAX_DEVICES];
  
  attempts   = 3;
  const uint64_t vendID_CERN       = 0x000000000000ce42;
  const uint32_t devID_PPS         = 0xde0d8ced;


  /* open EB socket and device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &socket0))               != EB_OK) {die(status, "failed to open Etherbone socket"); return 0;}
  if ((status = eb_device_open(socket0, netaddress0, EB_ADDR32 | EB_DATA32, attempts, &device0)) != EB_OK) {die(status, "failed to open Etherbone device"); return 0;}

  //get pps
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device0, vendID_CERN, devID_PPS, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  if (num_devices == 0) {
    fprintf(stderr, "%s: No wr pps found\n", program);
    return -1;
  }
  ppsAdr0 = (eb_address_t)(devices[0].sdb_component.addr_first);
  
  if(compare) {
    /* open EB socket and device */
    if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &socket1))               != EB_OK) {die(status, "failed to open Etherbone socket"); return 0;}
    if ((status = eb_device_open(socket1, netaddress1, EB_ADDR32 | EB_DATA32, attempts, &device1)) != EB_OK) {die(status, "failed to open Etherbone device"); return 0;}

    //get pps
    num_devices = MAX_DEVICES;
    if ((status = eb_sdb_find_by_identity(device1, vendID_CERN, devID_PPS, &devices[0], &num_devices)) != EB_OK)
    {return die(status, "failed to when searching for device");}
    if (num_devices == 0) {
      fprintf(stderr, "%s: No wr pps found\n", program);
      return -1;
    }
    ppsAdr1 = (eb_address_t)(devices[0].sdb_component.addr_first);
    
    //read WR State 2nd dev
    if ((status = eb_cycle_open(device1, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle1"); 
    eb_cycle_read(cycle, (eb_address_t)ppsAdr1 + PPS_STATE,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[3]);
    
    eb_cycle_read(cycle, (eb_address_t)ppsAdr1 + PPS_CNTR_UTCLO,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[5]);
    eb_cycle_read(cycle, (eb_address_t)ppsAdr1 + PPS_CNTR_UTCHI,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[4]);
    if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");
  }
  
  //read WR State 1st dev
  if ((status = eb_cycle_open(device0, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle0"); 
  eb_cycle_read(cycle, (eb_address_t)ppsAdr0 + PPS_STATE,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
  
  eb_cycle_read(cycle, (eb_address_t)ppsAdr0 + PPS_CNTR_UTCLO,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[2]);
  eb_cycle_read(cycle, (eb_address_t)ppsAdr0 + PPS_CNTR_UTCHI,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");
  
  wr_status[0] = (uint32_t)tmpRead[0] & 0x6;
  wr_utc_hi[0] = (uint32_t)tmpRead[1] & 0xff;
  wr_utc_lo[0] = (uint32_t)tmpRead[2];
  wr_now[0]    = (((uint64_t)wr_utc_hi[0]) << 32 | ((uint64_t)wr_utc_lo[0]));
  show(wr_now[0], wr_status[0], netaddress0);
  
  if(compare) {
  
    wr_status[1] = (uint32_t)tmpRead[3] & 0x6;
    wr_utc_hi[1] = (uint32_t)tmpRead[4] & 0xff;
    wr_utc_lo[1] = (uint32_t)tmpRead[5];
    wr_now[1]    = (((uint64_t)wr_utc_hi[1]) << 32 | ((uint64_t)wr_utc_lo[1]));
    show(wr_now[1], wr_status[1], netaddress1);
    printf("\n");
    if(wr_now[0] != wr_now[1]) {
      printf("%sDIFF DETECTED%s  Times differ by %s%llu%s seconds! (Note: 1s diff can be due to lag)\n", KRED, KNRM, KRED, (long long unsigned int)abs((int64_t)wr_now[0] - (int64_t)wr_now[1]), KNRM);
    } else {
      printf("%sNo Diff%s (Does not necessarily mean PPSs are aligned)\n", KGRN, KNRM);
    }
  }
  printf("\n");
  return 0;
}





