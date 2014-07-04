#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wait.h>
#include <etherbone.h>
#include "xmlaux.h"
#include "ftmx86.h"



#define MAX_DEVICES  100
#define PACKET_SIZE  500
#define CMD_LM32_RST 0x2


const char* program;
eb_device_t device;
eb_socket_t mySocket;

const uint32_t devID_Reset       = 0x3a362063;
const uint32_t devID_RAM         = 0x66cfeb52;
const uint64_t vendID_CERN       = 0x000000000000ce42;
const uint32_t devID_ClusterInfo = 0x10040086;
const uint64_t vendID_GSI        = 0x0000000000000651;
char           devName_RAM_pre[] = "WB4-BlockRAM_";

eb_data_t tmpRead[2];
      
volatile uint32_t embeddedOffset, resetOffset, inaOffset, actOffset, targetOffset;
uint8_t error, verbose, readonly;
volatile uint32_t cpuQty;

void ebRamOpen(const char* netaddress, uint8_t cpuId);
const uint8_t*  ebRamRead(uint32_t address, uint32_t len, const uint8_t* buf);
const uint8_t*  ebRamWrite(const uint8_t* buf, uint32_t address, uint32_t len);
void ebRamClose(void);
static int die(eb_status_t status, const char* what) {
  
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  exit(1);
}


static void strreverse(char* begin, char* end) {
   
   char aux;
   while(end>begin) aux=*end, *end--=*begin, *begin++=aux;
}
   
static void itoa(int value, char* str, int base) {
   
   static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
   char* wstr=str;
   int sign;
   div_t res;

   // Validate base
   if (base<2 || base>35){ *wstr='\0'; return; }
   
   // Take care of sign
   if ((sign=value) < 0) value = -value;
   
   // Conversion. Number is reversed.
   do {
      res = div(value,base);
      *wstr++ = num[res.rem];
      value = res.quot;
   }while(value);
   
   if(sign<0) *wstr++='-';
   *wstr='\0';
   
   // Reverse string
   strreverse(str,wstr-1);
}

static int getResetAdr()
{
  int idx = 0;
  int num_devices;
  struct sdb_device devices[MAX_DEVICES]; 
  
  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_Reset, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no reset controller found\n", program);
    return 0xDEADBEEF;
  }

  if (num_devices > MAX_DEVICES) {
    fprintf(stderr, "%s: more devices found that tool supports (%d > %d)\n", program, num_devices, MAX_DEVICES);
    return 0xDEADBEEF;
  }

  if (idx > num_devices) {
    fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
    return 0xDEADBEEF;
  }

 return devices[0].sdb_component.addr_first;
}

void ebRamOpen(const char* netaddress, uint8_t cpuId)
{
   eb_cycle_t cycle;
   eb_status_t status;
   int idx;
   int attempts;
   int num_devices;
   struct sdb_device devices[MAX_DEVICES];
   char              devName_RAM_post[4];
   
   
   attempts   = 3;
   idx        = -1;

   /* open EB socket and device */
   if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &mySocket))               != EB_OK) die(status, "failed to open Etherbone socket");
   if ((status = eb_device_open(mySocket, netaddress, EB_ADDR32 | EB_DATA32, attempts, &device)) != EB_OK) die(status, "failed to open Etherbone device");

   num_devices = MAX_DEVICES;
   if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_ClusterInfo, &devices[0], &num_devices)) != EB_OK)
   die(status, "failed to when searching for device");
   if (num_devices == 0) {
      fprintf(stderr, "%s: No lm32 clusterId rom found\n", program);
      goto error;
   }

   if (num_devices > MAX_DEVICES) {
      fprintf(stderr, "%s: Way too many lm32 clusterId roms found, something's wrong\n", program);
      goto error;
   }

   if (idx > num_devices) {
      fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
      goto error;
   }
      
   //get number of CPUs and create search string
   status = eb_device_read(device, (eb_address_t)devices[0].sdb_component.addr_first, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0], 0, eb_block);
   if (status != EB_OK) die(status, "failed to create cycle");
   cpuQty = (uint8_t)tmpRead[0];
   
   devName_RAM_post[0] = '0';
   devName_RAM_post[1] = '0' + (cpuQty & 0xf);
   devName_RAM_post[2] = '0' + (cpuId  & 0xf);
   devName_RAM_post[3] =  0;

   if(cpuQty <= cpuId)
   {   
      fprintf(stderr, "The CpuId you gave me (%u) is higher than maximum (%u-1).\n", cpuId, cpuQty);
      goto error;
   }
  
   num_devices = MAX_DEVICES;
   if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
   die(status, "failed to when searching for device");
   if (num_devices == 0) {
      fprintf(stderr, "%s: no RAM's found\n", program);
      goto error;
   }

   if (num_devices > MAX_DEVICES) {
      fprintf(stderr, "%s: more devices found that tool supports (%d > %d)\n", program, num_devices, MAX_DEVICES);
      goto error;
   }

   if (idx > num_devices) {
      fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
      goto error;
   }
   if (idx == -1) {
      //printf("Found %u devs\n", num_devices);
      for (idx = 0; idx < num_devices; ++idx) {
         if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0)
         {
            embeddedOffset = devices[idx].sdb_component.addr_first;
         }
      }
   } else {
      printf("0x%"PRIx64"\n", devices[idx].sdb_component.addr_first);
      embeddedOffset = devices[idx].sdb_component.addr_first;
   }

   // get the active and inactive pointer value from the core

   if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
   eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + FTM_PACT_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
   eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + FTM_PINA_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
   if ((status = eb_cycle_close(cycle)) != EB_OK) die(status, "failed to close read cycle");
   
   actOffset = (uint32_t) tmpRead[0];
   inaOffset = (uint32_t) tmpRead[1];
   
   return;
   
   error:
   ebRamClose();
   exit(1);
}

void ebRamClose()
{

   eb_status_t status;

   if ((status = eb_device_close(device))   != EB_OK) die(status, "failed to close Etherbone device");
   if ((status = eb_socket_close(mySocket)) != EB_OK) die(status, "failed to close Etherbone socket");
}

const uint8_t* ebRamRead(uint32_t address, uint32_t len, const uint8_t* buf)
{
   
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start;
   uint32_t* readin = (uint32_t*)buf;
   eb_data_t tmpReadin[BUF_SIZE/2];   

   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)  die(status, "failed to create cycle");
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         //printf("%4u %08x -> %p\n", i, (address+(i<<2)), &readin[i]);
         eb_cycle_read(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, &tmpReadin[i]);
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close read cycle");
      for(i= start>>2; i< (start + partLen) >>2;i++) readin[i] = (uint32_t)tmpReadin[i]; //this is important caus eb_data_t is 64b wide!
      start = start + partLen;
   }
      
   return buf;
}

const uint8_t* ebRamWrite(const uint8_t* buf, uint32_t address, uint32_t len)
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start;
   uint32_t* writeout = (uint32_t*)buf;   
   
   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)writeout[i]); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close write cycle");
      start = start + partLen;
   }
   return buf;
}

static void help(void) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> [command]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -c <core-idx>             select a core by index, -1 selects all\n");
  fprintf(stderr, "  -v                        verbose operation, print more details\n");
  fprintf(stderr, "  -h                        display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  status                    (default) report core status\n");
  fprintf(stderr, "  run                       start this core\n");
  fprintf(stderr, "  stop                      request stop on this core\n");
  fprintf(stderr, "  fstop                     force stop on this core\n");
  fprintf(stderr, "  bpset                     set branchpoint. accepts 0..n or 'idle'\n");
  fprintf(stderr, "  idle                      request idle state on this core\n");
  fprintf(stderr, "  swap                      swap active and inactive page on this core\n");
  fprintf(stderr, "  put    <filename>         puts ftm data from xml file to inactive page on this core\n");
  fprintf(stderr, "  get                       gets ftm data from inactive page and displays it\n");
  fprintf(stderr, "  dump                      gets ftm data from active page and displays it\n");
  fprintf(stderr, "  loadfw <filename>         puts firmware from bin file to core\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  condump                   debug feature. make this core output its data to console\n");
  //fprintf(stderr, "  condget                   debug feature. if this core is wating for a condition value, ask what it is\n");
  //fprintf(stderr, "  condset                   debug feature. set condition value in shared mem for this core\n");
  fprintf(stderr, "\n");
}


static void status(uint8_t cpuId)
{
       uint32_t ftmStatus;
       eb_status_t status;
       
    if ((status = eb_device_read(device, embeddedOffset + FTM_STAT_OFFSET, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0], 0, eb_block)) != EB_OK)
    die(status, "failed to read");  
    ftmStatus = (uint32_t) tmpRead[0];
    printf("***********************************************************************************\n");
    printf("** Core #%u: Status: %08x MsgCnt: %8u                                    **\n", cpuId, ftmStatus, (ftmStatus >> 16));
    printf("***********************************************************************************\n");
    printf("-----------------------------------------------------------------------------------\n");
    if(ftmStatus & STAT_RUNNING)    printf("RUNNING  \t");  else printf("    -    \t");
    if(ftmStatus & STAT_IDLE)       printf("IDLE     \t");  else printf("    -    \t");
    if(ftmStatus & STAT_STOP_REQ)   printf("STOP_REQ \t");  else printf("    -    \t");
    if(ftmStatus & STAT_ERROR)      printf("ERROR    \t");  else printf("    -    \t");
    if(ftmStatus & STAT_WAIT)       printf("WAIT_COND\t");  else printf("    -    \t");
    printf("\n");
    printf("-----------------------------------------------------------------------------------\n");
    if(actOffset < inaOffset) printf("  Active Page: A @ 0x%08x\nInactive Page: B @ 0x%08x\n", actOffset, inaOffset);
    else                      printf("  Active Page: B @ 0x%08x\nInactive Page: A @ 0x%08x\n", actOffset, inaOffset);
    printf("-----------------------------------------------------------------------------------\n");
    
}

int main(int argc, char** argv) {

   
   int opt;
   char *value_end;
   const char* netaddress, *command;
   
   uint8_t  bufWrite[BUF_SIZE];
   uint8_t  bufRead[BUF_SIZE];
   char     filename[64];
   char     bpstr[10];
   
   t_ftmPage*  pPage    = NULL;
   t_ftmPage*  pNewPage = NULL;
   uint8_t*    pBufRead = &bufRead[0];
   uint8_t*    pBufWrite = &bufWrite[0];
   
   memset(pBufWrite, 0, BUF_SIZE);
   memset(pBufRead, 0, BUF_SIZE);
   
   int cpuId;
   uint8_t firstCpu, lastCpu;
   
   eb_status_t ebstatus;
   
   cpuId    = 0;
   error    = 0;
   verbose  = 0;
   readonly = 1;
   program  = argv[0];
   
   

   while ((opt = getopt(argc, argv, "c:vh")) != -1) {
      switch (opt) {
         
         case 'c':
            cpuId = strtol(optarg, &value_end, 0);
            if (*value_end || cpuId < -1 ||cpuId > 32) {
              fprintf(stderr, "%s: invalid cpu id -- '%s'\n", program, optarg);
              error = 1;
            }
         break;
         case 'v':
            verbose = 1;
            break;
         case 'h':
            help();
            return 0;
         case ':':
         case '?':
            error = 1;
            break;
         default:
            fprintf(stderr, "%s: bad getopt result\n", program);
            return 1;
      }
   }

   if (error) return 1;

   if (optind >= argc) {
   fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
   fprintf(stderr, "\n");
   help();
   return 1;
   }
   
   netaddress = argv[optind];
   printf("\n");

   
   if (optind+1 < argc)  command = argv[++optind];
   else                 {command = "status"; cpuId = -1;}
   
   if ( (!strcasecmp(command, "put")) || (!strcasecmp(command, "loadfw"))  )
   {
      if (optind+1 < argc) {
         strncpy(filename, argv[optind+1], 64);
         readonly = 0;
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <filename>\n", program);
         return 1;
      }
   }
   
   if (!strcasecmp(command, "setbp")) 
   {
      if (optind+1 < argc) {
         bpstr[9] = 0;
         strncpy(bpstr, argv[optind+1], 8);
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <branchpoint name>\n", program);
         return 1;
      }
   }
   
   ebRamOpen(netaddress, 0);
   
   if (!strcasecmp(command, "loadfw")) {
      resetOffset = getResetAdr();
      printf("Putting CPU into Reset for FW load\n");
      ebstatus = eb_device_write(device, resetOffset, EB_BIG_ENDIAN | EB_DATA32, CMD_LM32_RST, 0, eb_block);
      if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");
   }
   ebRamClose();
   
   if(cpuId < 0) { firstCpu   = 0; 
                   lastCpu    = cpuQty-1;
                  } // bit wasteful but safer than self modifiying loop
   else { firstCpu = (uint8_t)cpuId; 
          lastCpu  = (uint8_t)cpuId;
        }    
   
   if (!strcasecmp(command, "status")) { printf("#### FTM @ %s ####\n", netaddress); }
   
   uint8_t k;
   for(k = firstCpu; k <= lastCpu; k++)
   {
      if(strcasecmp(command, "loadfw"))
      {
         ebRamOpen(netaddress, k); // open connection if the command is NOT loadfw
      
         if(!(actOffset + inaOffset)) {
            fprintf(stderr, "CPU #%u NOT INITIALIZED!\nrun '%s %s -c  %u loadfw <firmware.bin>' to load fw to this core\n", k, program, netaddress, k);
            fprintf(stderr, "or  '%s %s -c -1 loadfw <firmware.bin>' to load fw to all core\n", program, netaddress);
            return -1;  
         }
      }
       
      
      /* -------------------------------------------------------------------- */
     if (!strcasecmp(command, "status")) {
       status(k);
     }

     /* -------------------------------------------------------------------- */
     else if (!strcasecmp(command, "run")) {
       
       if (verbose) {
         printf("Starting FTM Core %u\n", k);
       }
       ebstatus = eb_device_write(device, embeddedOffset + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_START, 0, eb_block);
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");

     }
     
     else if (!strcasecmp(command, "stop")) {
       
       if (verbose) {
         printf("Requesting FTM Core %u to stop\n", k);
       }
       ebstatus = eb_device_write(device, embeddedOffset + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_STOP_REQ, 0, eb_block);
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");
     }
     
     else if (!strcasecmp(command, "idle")) {
       
       if (verbose) {
         printf("Setting BP of FTM COre %u to idle\n", k);
       }
       ebstatus = eb_device_write(device, embeddedOffset + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_IDLE, 0, eb_block);
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");

     }
     
     else if (!strcasecmp(command, "fstop")) {
       
       if (verbose) {
         printf("Forcing FTM Core %u to stop\n", k);
       }
       ebstatus = eb_device_write(device, embeddedOffset + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_STOP_NOW, 0, eb_block);
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");
     } 
     
     else if (!strcasecmp(command, "reset")) {
       if (verbose) {
         printf("Resetting FTM Core %u\n", k);
       }
       ebstatus = eb_device_write(device, embeddedOffset + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_RST, 0, eb_block);
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 
     }
     
     else if (!strcasecmp(command, "setbp")) {
       int planIdx, planQty;
       eb_cycle_t cycle;
       uint32_t bp;
       
      if(!strcasecmp(bpstr, "idle")) planIdx = -1;
      else {planIdx = strtol(bpstr, 0, 10);}   
      
      //user gave us a planIdx. load corresponding ptr
      if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(ebstatus, "failed to create cycle"); 
      eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + actOffset + FTM_PAGE_PLANQTY_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      //if the user wanted to go to idle, read idle ptr from interface, else read plan ptr
      if(planIdx == -1)
      {eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + FTM_IDLE_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      else 
      {eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + actOffset + FTM_PAGE_PLANS_OFFSET + 4*planIdx), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      if((ebstatus = eb_cycle_close(cycle)) != EB_OK) die(ebstatus, "failed to close read cycle"); 
  
      planQty  = (uint32_t)tmpRead[0];
      bp       = (uint32_t)tmpRead[1];
      // Check and write to BP
      if(bp != FTM_NULL && planIdx < planQty) 
      {
         printf("Writing plan %d @ 0x%08x to BP\n", planIdx, bp);
         ebstatus = eb_device_write(device, (uint32_t)embeddedOffset + actOffset + FTM_PAGE_BP_OFFSET, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)bp, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");  
      } else
      { 
         if (planIdx >= planQty) printf ("Sorry, but the plan index is neither idle nor 0 <= %d (planIdx) <  %u (planQty)\n", planIdx, planQty);
         else printf ("Found a NULL ptr at plan idx %d, something is wrong\n", planIdx);
      }
       
     }
    
      
     else if (!strcasecmp(command, "loadfw")) {

            char cOffs[11] = "0x00000000";
            char cmdStr[80];
            
            //get embeddedOffset.
            //TODO: replace offset retrieval code with something more elegant 
            ebRamOpen(netaddress, k);
            ebRamClose();
            
            itoa(embeddedOffset, &cOffs[2], 16);
            strcpy (cmdStr, "eb-put ");
            strcat (cmdStr, netaddress);
            strcat (cmdStr, " ");
            strcat (cmdStr, cOffs);
            strcat (cmdStr, " ");
            strcat (cmdStr, filename);
            
            if (verbose) {
               printf("Loading %s to CPU %u @ %s\n", filename, k, cOffs);  
            } else printf("Loading Firmware to CPU %u\n", k);
            
            FILE* ebput = popen(cmdStr, "w");
            pclose(ebput);
            
     }
     else if (!strcasecmp(command, "swap")) {
       
       if (verbose) {
         printf("Swapping Active/Inactive page on CPU %u\n", k);
       }
       
       ebstatus = eb_device_write(device, (eb_address_t)(embeddedOffset + FTM_CMD_OFFSET), EB_BIG_ENDIAN | EB_DATA32, 
                                    (eb_data_t)CMD_COMMIT_PAGE, 0, eb_block);
      if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 

     } 
     
     else if (!strcasecmp(command, "condump")) {
       
       if (verbose) {
         printf("Commanding FTM CPU %u to show FTM Data on console\n", k);
       }
       ebstatus = eb_device_write(device, embeddedOffset + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_SHOW_ACT, 0, eb_block) ;
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 
       sleep(1); 
     } 
     
     else if ((!strcasecmp(command, "put")) || (!strcasecmp(command, "get")) || (!strcasecmp(command, "dump"))) {
       
       targetOffset = inaOffset;
       
       if(!strcasecmp(command, "put")) {
          if(!readonly) {
            printf("Parsing %s ...", filename);
            pPage = parseXml(filename);
            printf("done.\n");
            if(verbose) showFtmPage(pPage);
            serPage (pPage, pBufWrite, targetOffset, k);
            ebRamWrite(pBufWrite, embeddedOffset + targetOffset, BUF_SIZE);
            printf("Writing %u bytes FTM Data to 0x%08x...", BUF_SIZE, embeddedOffset + targetOffset);
            printf("done.\n");
         } else fprintf(stderr, "No xml file specified\n");
         
      }
      if(!strcasecmp(command, "dump")) targetOffset = actOffset;
      ebRamRead(embeddedOffset + targetOffset, BUF_SIZE, pBufRead);
      printf("Reading %u bytes FTM Data from 0x%08x...", BUF_SIZE, embeddedOffset + targetOffset);
      printf("done.\n");
      
      //verify
      if(!readonly) {
         uint32_t i;
         for(i = 0; i<BUF_SIZE; i++) {
            if(!(pBufRead[i] == pBufWrite[i])) { 
               fprintf(stderr, "!ERROR! Verify of written ftmpage failed at offset 0x%08x\n", embeddedOffset + targetOffset +( i & ~0x3) );
               exit(1);
            }
         }
         printf("Verify OK.\n\n");   
      }
      
      pNewPage = deserPage(calloc(1, sizeof(t_ftmPage)), pBufRead, targetOffset);
      if(pNewPage != NULL)
      {  
         printf("Deserialization successful.\n\n");
         if(verbose || (!strcasecmp(command, "get")) || (!strcasecmp(command, "dump")) ) showFtmPage(pNewPage);
      }   
      else printf("Deserialization FAILED! Corrupt/No Data ?\n");
    }   
    else  printf("Unknown command: %s\n", command);  
  
    if(strcasecmp(command, "loadfw")) ebRamClose(); // close connection if command is NOT loadfw
    
   } 
      
   if(!strcasecmp(command, "loadfw")) {
      ebRamOpen(netaddress, 0);
      printf("Releasing all CPUs from Reset\n\n");
      ebstatus = eb_device_write(device, resetOffset, EB_BIG_ENDIAN | EB_DATA32, 0, 0, eb_block);
      if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 
      ebRamClose(); 
    }
   return 0;
}


