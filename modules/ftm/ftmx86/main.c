#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wait.h>
#include <etherbone.h>
#include "access.h"
#include "xmlaux.h"
#include "ftmx86.h"
#include "fancy.h"


#define FILENAME_LEN   256
#define UINT64_STR_LEN 24




const char* program;
eb_device_t device;
eb_socket_t mySocket;
t_ftmAccess ftmAccess;
t_ftmAccess* pAccess = &ftmAccess;


extern bool bigEndian;

#define EBM_REG_CLEAR         0                         
#define EBM_REG_FLUSH         (EBM_REG_CLEAR        +4)        
#define EBM_REG_STATUS        (EBM_REG_FLUSH        +4)         
#define EBM_REG_SRC_MAC_HI    (EBM_REG_STATUS       +4)       
#define EBM_REG_SRC_MAC_LO    (EBM_REG_SRC_MAC_HI   +4)    
#define EBM_REG_SRC_IPV4      (EBM_REG_SRC_MAC_LO   +4)    
#define EBM_REG_SRC_UDP_PORT  (EBM_REG_SRC_IPV4     +4)   
#define EBM_REG_DST_MAC_HI    (EBM_REG_SRC_UDP_PORT +4)  
#define EBM_REG_DST_MAC_LO    (EBM_REG_DST_MAC_HI   +4)   
#define EBM_REG_DST_IPV4      (EBM_REG_DST_MAC_LO   +4)  
#define EBM_REG_DST_UDP_PORT  (EBM_REG_DST_IPV4     +4)   
#define EBM_REG_MTU           (EBM_REG_DST_UDP_PORT +4)  
#define EBM_REG_ADR_HI        (EBM_REG_MTU          +4)    
#define EBM_REG_OPS_MAX       (EBM_REG_ADR_HI       +4) 
#define EBM_REG_EB_OPT        (EBM_REG_OPS_MAX      +4) 
#define EBM_REG_LAST          (EBM_REG_EB_OPT)

// Priority Queue RegisterLayout
static const struct {
   uint32_t rst;
   uint32_t force;
   uint32_t dbgSet;
   uint32_t dbgGet;
   uint32_t clear;
   uint32_t cfgGet;
   uint32_t cfgSet;
   uint32_t cfgClr;
   uint32_t dstAdr;
   uint32_t heapCnt;
   uint32_t msgCntO;
   uint32_t msgCntI;
   uint32_t tTrnHi;
   uint32_t tTrnLo;
   uint32_t tDueHi;
   uint32_t tDueLo;
   uint32_t capacity;
   uint32_t msgMax;
   uint32_t ebmAdr;
   uint32_t tsAdr;
   uint32_t tsCh;
   uint32_t cfg_ENA;
   uint32_t cfg_FIFO;    
   uint32_t cfg_IRQ;
   uint32_t cfg_AUTOPOP;
   uint32_t cfg_AUTOFLUSH_TIME;
   uint32_t cfg_AUTOFLUSH_MSGS;
   uint32_t cfg_MSG_ARR_TS;
   uint32_t force_POP;
   uint32_t force_FLUSH;
} r_FPQ = {    .rst        =  0x00,
               .force      =  0x04,
               .dbgSet     =  0x08,
               .dbgGet     =  0x0c,
               .clear      =  0x10,
               .cfgGet     =  0x14,
               .cfgSet     =  0x18,
               .cfgClr     =  0x1C,
               .dstAdr     =  0x20,
               .heapCnt    =  0x24,
               .msgCntO    =  0x28,
               .msgCntI    =  0x2C,
               .tTrnHi     =  0x30,
               .tTrnLo     =  0x34,
               .tDueHi     =  0x38,
               .tDueLo     =  0x3C,
               .capacity   =  0x40,
               .msgMax     =  0x44,
               .ebmAdr     =  0x48,
               .tsAdr      =  0x4C,
               .tsCh       =  0x50,
               .cfg_ENA             = 1<<0,
               .cfg_FIFO            = 1<<1,    
               .cfg_IRQ             = 1<<2,
               .cfg_AUTOPOP         = 1<<3,
               .cfg_AUTOFLUSH_TIME  = 1<<4,
               .cfg_AUTOFLUSH_MSGS  = 1<<5,
               .cfg_MSG_ARR_TS      = 1<<6,
               .force_POP           = 1<<0,
               .force_FLUSH         = 1<<1
};

char           devName_RAM_pre[] = "WB4-BlockRAM_";

eb_data_t tmpRead[3];
      
volatile uint32_t targetOffset, clusterOffset;
uint8_t error, verbose, readonly;



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



static uint32_t bytesToUint32(uint8_t* pBuf)
{
   uint8_t i;
   uint32_t val=0;
   
   for(i=0;i<FTM_WORD_SIZE;   i++) val |= (uint32_t)pBuf[i] << (8*i);
   return val;
}

static void ebPeripheryStatus()
{
  uint8_t buff[2048];
  uint32_t cfg;
  uint64_t tmp;
   
  ebRamRead(pAccess->ebmAdr + EBM_REG_STATUS, EBM_REG_LAST, &buff[EBM_REG_STATUS]);
  printf ("EBM||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  printf ("Status\t\t: 0x%08x\n",  bytesToUint32(&buff[EBM_REG_STATUS]));
  printf ("Src Mac\t\t: 0x%08x%04x\n", bytesToUint32(&buff[EBM_REG_SRC_MAC_HI]),  bytesToUint32(&buff[EBM_REG_SRC_MAC_LO]));
  printf ("Src IP\t\t: 0x%08x\n", bytesToUint32(&buff[EBM_REG_SRC_IPV4]));
  printf ("Src Port\t: 0x%04x\n\n", bytesToUint32(&buff[EBM_REG_SRC_UDP_PORT])); 
  printf ("Dst Mac\t\t: 0x%08x%04x\n", bytesToUint32(&buff[EBM_REG_DST_MAC_HI]),  bytesToUint32(&buff[EBM_REG_DST_MAC_LO]));
  printf ("Dst IP\t\t: 0x%08x\n", bytesToUint32(&buff[EBM_REG_DST_IPV4]));
  printf ("Dst Port\t: 0x%04x\n\n", bytesToUint32(&buff[EBM_REG_DST_UDP_PORT]));
  printf ("MTU\t\t: %u\n", bytesToUint32(&buff[EBM_REG_MTU]));
  printf ("Adr Hi\t\t: 0x%08x\n", bytesToUint32(&buff[EBM_REG_ADR_HI]));
  printf ("Ops Max\t\t: %u\n", bytesToUint32(&buff[EBM_REG_OPS_MAX]));
  printf ("EB Opt\t\t: 0x%08x\n\n", bytesToUint32(&buff[EBM_REG_EB_OPT]));

  ebRamRead(pAccess->prioQAdr + r_FPQ.cfgGet, 4, &buff[r_FPQ.cfgGet]);
  ebRamRead(pAccess->prioQAdr + r_FPQ.dstAdr, r_FPQ.ebmAdr - r_FPQ.dstAdr +4, &buff[r_FPQ.dstAdr]);
  printf ("FPQ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  cfg = bytesToUint32(&buff[r_FPQ.cfgGet]);
  printf("-----------------------------------------------------------------------------------\n");
  if(cfg & r_FPQ.cfg_ENA)            printf("    ENA   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_FIFO)           printf("   FIFO   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOPOP)        printf("   APOP   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOFLUSH_TIME) printf(" AFL_TIME ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOFLUSH_MSGS) printf(" AFL_MSGS ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_MSG_ARR_TS)     printf("  TS_ARR  ");  else printf("     -    ");
  printf("\n");
  printf("-----------------------------------------------------------------------------------\n");
  printf ("Dst Adr\t\t: 0x%08x\n\n", bytesToUint32(&buff[r_FPQ.dstAdr]));
  printf ("Heap Cnt\t: %u\n", bytesToUint32(&buff[r_FPQ.heapCnt]));
  printf ("msg CntO\t: %u\n", bytesToUint32(&buff[r_FPQ.msgCntO]));
  printf ("msg CntI\t: %u\n\n", bytesToUint32(&buff[r_FPQ.msgCntI]));  
  tmp = (((uint64_t)bytesToUint32(&buff[r_FPQ.tTrnHi])) <<32) + ((uint64_t)bytesToUint32(&buff[r_FPQ.tTrnLo]));
  printf ("TTrn\t\t: %llu\n", (long long unsigned int)tmp<<3);
  tmp = (((uint64_t)bytesToUint32(&buff[r_FPQ.tDueHi])) <<32) + ((uint64_t)bytesToUint32(&buff[r_FPQ.tDueLo]));
  printf ("TDue\t\t: %llu\n\n", (long long unsigned int)tmp<<3);
  printf ("Capacity\t: %u\n", bytesToUint32(&buff[r_FPQ.capacity]));
  printf ("msg max\t\t: %u\n\n", bytesToUint32(&buff[r_FPQ.msgMax]));
  printf ("EBM Adr\t\t: 0x%08x\n", bytesToUint32(&buff[r_FPQ.ebmAdr]));
  printf ("ts Adr\t\t: 0x%08x\n", bytesToUint32(&buff[r_FPQ.tsAdr]));
  printf ("ts Ch\t\t: 0x%08x\n\n", bytesToUint32(&buff[r_FPQ.tsCh]));
   
   
   return;
   
}



static void help(void) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> [command]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  duetime <Time / ns>       Set due time for priority queue\n");
  fprintf(stderr, "  trntime <Time / ns>       Set transmission delay for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> Set maximum messages in a packet for priority queue\n\n");
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
  fprintf(stderr, "  clear                     clears all pages on this core\n");
  fprintf(stderr, "  get                       gets ftm data from inactive page and displays it\n");
  fprintf(stderr, "  dump                      gets ftm data from active page and displays it\n");
  fprintf(stderr, "  loadfw <filename>         puts firmware from bin file to core\n");
  fprintf(stderr, "  condump                   debug feature. make this core output its data to console\n");
  fprintf(stderr, "  preptime <Time / ns>      Set preparation time on this core\n");
  fprintf(stderr, "\n");

  //fprintf(stderr, "  condget                   debug feature. if this core is wating for a condition value, ask what it is\n");
  //fprintf(stderr, "  condset                   debug feature. set condition value in shared mem for this core\n");
  fprintf(stderr, "\n");
}


static void status(uint8_t cpuIdx)
{
  uint32_t ftmStatus, mySharedMem, sharedMem;
  eb_status_t status;
  eb_data_t tmpRd[4];
  eb_cycle_t cycle;
  eb_address_t tmpAdr = pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs;
  long long unsigned int ftmTPrep;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_STAT_OFFSET,         EB_BIG_ENDIAN | EB_DATA32, &tmpRd[0]);
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_SHARED_PTR_OFFSET,   EB_BIG_ENDIAN | EB_DATA32, &tmpRd[1]); 
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET,        EB_BIG_ENDIAN | EB_DATA32, &tmpRd[2]);
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET+4,      EB_BIG_ENDIAN | EB_DATA32, &tmpRd[3]);
  if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close read cycle");
  ftmStatus = (uint32_t) tmpRd[0];
  sharedMem = tmpRd[1]; //convert lm32's view to pcie's view
  mySharedMem = pAccess->clusterAdr + (sharedMem & 0x3fffffff); //convert lm32's view to pcie's view
  ftmTPrep = (long long unsigned int)(((uint64_t)tmpRd[2]) << 32 | ((uint64_t)tmpRd[3]));

  uint8_t i;
     
  printf("\u2552"); for(i=0;i<79;i++) printf("\u2550"); printf("\u2555\n");
  printf("\u2502 %sCore #%02u%s                                                                      \u2502\n", KCYN, cpuIdx, KNRM);
  printf("\u251C"); for(i=0;i<24;i++) printf("\u2500"); printf("\u252C"); for(i=0;i<54;i++) printf("\u2500"); printf("\u2524\n");
  printf("\u2502 Status: %02x ErrCnt: %3u \u2502   MsgCnt: %9u       TPrep: %13llu ns    \u2502\n", \
   (uint8_t)ftmStatus, (uint8_t)(ftmStatus >> 8), (uint16_t)(ftmStatus >> 16), ftmTPrep<<3);
  printf("\u251C"); for(i=0;i<24;i++) printf("\u2500"); printf("\u253C"); for(i=0;i<54;i++) printf("\u2500"); printf("\u2524\n");
  printf("\u2502 Shared Mem: 0x%08x \u2502", mySharedMem + cpuIdx*0x0C);
  if(pAccess->pCores[cpuIdx].actOffs < pAccess->pCores[cpuIdx].inaOffs) printf("   Act Page: A 0x%08x  Inact Page: B 0x%08x", pAccess->pCores[cpuIdx].actOffs, pAccess->pCores[cpuIdx].inaOffs);
  else                      printf("   Act Page: B 0x%08x  Inact Page: A 0x%08x", pAccess->pCores[cpuIdx].actOffs, pAccess->pCores[cpuIdx].inaOffs);
  printf("   \u2502\n");
  printf("\u251C"); for(i=0;i<24;i++) printf("\u2500"); printf("\u2534"); for(i=0;i<54;i++) printf("\u2500"); printf("\u2524\n");
  printf("\u2502       ");

  if(ftmStatus & STAT_RUNNING)    printf("   %sRUNNING%s   ", KGRN, KNRM);  else printf("   %sSTOPPED%s   ", KRED, KNRM);
  if(ftmStatus & STAT_IDLE)       printf("     %sIDLE%s    ", KYEL, KNRM);  else printf("     %sBUSY%s    ", KGRN, KNRM);
  if(ftmStatus & STAT_STOP_REQ)   printf("   STOP_REQ  ");  else printf("      -      ");
  if(ftmStatus & STAT_ERROR)      printf("     %sERROR%s   ", KRED, KNRM);  else printf("     %sOK%s      ", KGRN, KNRM);
  if(ftmStatus & STAT_WAIT)       printf("  WAIT_COND  ");  else printf("      -      ");
  printf("       \u2502\n");
  printf("\u2514"); for(i=0;i<79;i++) printf("\u2500"); printf("\u2518\n");

}

int main(int argc, char** argv) {

   
   int opt;
   char *value_end;
   const char* netaddress, *command;
   uint32_t resetBits;
   
   uint8_t  bufWrite[BUF_SIZE];
   uint8_t  bufRead[BUF_SIZE];
   
   char     filename[FILENAME_LEN];
   char     filename2[FILENAME_LEN];
   char     bpstr[10];
   uint64_t uint64val = 0;
   
   t_ftmPage*  pPage    = NULL;
   t_ftmPage*  pNewPage = NULL;
   uint8_t*    pBufRead = &bufRead[0];
   uint8_t*    pBufWrite = &bufWrite[0];
   
   memset(pBufWrite, 0, BUF_SIZE);
   memset(pBufRead, 0, BUF_SIZE);
   
   int cpuId;
   uint8_t firstCpu, lastCpu, overrideFWcheck;
   
   eb_status_t ebstatus;
   eb_cycle_t  cycle;
   
   cpuId    = 0;
   error    = 0;
   verbose  = 0;
   readonly = 1;
   overrideFWcheck = 0;
   program  = argv[0];
   
  

   while ((opt = getopt(argc, argv, "c:ovh")) != -1) {
      switch (opt) {
         case 'o':
            overrideFWcheck = 1;
            break;
         case 'v':
            verbose = 1;
            break;
         case 'c':
            cpuId = strtol(optarg, &value_end, 0);
            if (*value_end || cpuId < -1 ||cpuId > 32) {
              fprintf(stderr, "%s: invalid cpu id -- '%s'\n", program, optarg);
              error = 1;
            }
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
   
     
   
   if ( (!strcasecmp(command, "put")) || (!strcasecmp(command, "loadfw")))
   {
      if (optind+1 < argc) {
         strncpy(filename, argv[optind+1], FILENAME_LEN);
         readonly = 0;
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <filename>\n", program);
         return 1;
      }
   }
   
   if ( (!strcasecmp(command, "makefw")) )
   {
      if (optind+2 < argc) {
         bigEndian = false;
         strncpy(filename, argv[optind+1], FILENAME_LEN);
         strncpy(filename2, argv[optind+2], FILENAME_LEN);
         
         FILE *src = fopen(filename2, "r");
         FILE *dst = fopen("testfw.bin", "w");
         pPage = parseXmlFile(filename);
         if(verbose) showFtmPage(pPage);
         serPage (pPage, pBufWrite, ftm_shared_offs, cpuId);
       
         size_t l1;
         unsigned char buffer[8192];
         unsigned int  numr, numw;
          
         while(feof(src)==0){	
	         if((numr=fread(buffer,1,8192,src))>8192){
		         if(ferror(src)!=0){
		            fprintf(stderr,"read file error.\n");
		            exit(1);
		         }
		         else if(feof(src)!=0);
	         } else printf("Read %u bytes\n", numr);
	         if((numw=fwrite(buffer,1,numr,dst))!=numr){
		         fprintf(stderr,"write file error.\n");
		         exit(1);
	         }
	      }	
	
	
         fclose(src);
         printf("Writing to offset %08x\n", ftm_shared_offs);
         fseek(dst, ftm_shared_offs, 0);
         fwrite(pBufWrite, 1, FTM_PAGESIZE, dst);     
         fclose(dst); 
      
         return 0;
         
      } else {
         fprintf(stderr, "%s: expecting two non-optional arguments: <fw filename> <xml filename>\n", program);
         return 1;
      }
   }
   
   
   if ( (!strcasecmp(command, "preptime")) || (!strcasecmp(command, "duetime")) || (!strcasecmp(command, "trntime")) || (!strcasecmp(command, "maxmsg"))) { 
      if (optind+1 < argc) {
         long long unsigned int tmp = strtoll(argv[optind+1], NULL, 10);
         if(!strcasecmp(command, "maxmsg"))  uint64val = (uint64_t)tmp;
         else                                uint64val = (uint64_t)(tmp>>3);
      } else {
         if(!strcasecmp(command, "maxmsg")) fprintf(stderr, "%s: expecting one non-optional argument: <Message Quantity>\n", program);
         else fprintf(stderr, "%s: expecting one non-optional argument: <Time / ns>\n", program);
         return 1;
      }
   }
   
   if (!strcasecmp(command, "setbp")) {
      if (optind+1 < argc) {
         bpstr[9] = 0;
         strncpy(bpstr, argv[optind+1], 8);
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <branchpoint name>\n", program);
         return 1;
      }
   }
   
//*****************************************************************************************************************//

  printf("Connecting to FTM\n");
  openFtm(netaddress, pAccess, overrideFWcheck);

  //op for one CPU or all?
  if(cpuId < 0) { firstCpu   = 0; 
    lastCpu    = pAccess->cpuQty-1;
  } else { firstCpu = (uint8_t)cpuId; 
    lastCpu  = (uint8_t)cpuId;
  }  
   
  if(cpuId < 0) {
    resetBits = (1 << pAccess->cpuQty) -1;
  } else {
    resetBits = 1 << firstCpu;
  }
   
   if(!strcasecmp(command, "duetime")) {
         if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(ebstatus, "failed to create cycle"); 
         eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tDueHi), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)(uint64val>>32)));
         eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tDueLo), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)uint64val));
         if ((ebstatus = eb_cycle_close(cycle)) != EB_OK)  die(ebstatus, "failed to close write cycle");
         return 0;
   }
   
   
   if(!strcasecmp(command, "trntime")) 
   {
         if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(ebstatus, "failed to create cycle"); 
         eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tTrnHi), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)(uint64val>>32)));
         eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tTrnLo), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)uint64val));
         if ((ebstatus = eb_cycle_close(cycle)) != EB_OK)  die(ebstatus, "failed to close write cycle");
         return 0;
   }
   
   if(!strcasecmp(command, "maxmsg")) 
   {
         ebstatus = eb_device_write(device, (eb_address_t)(pAccess->prioQAdr + r_FPQ.msgMax), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)uint64val), 0, eb_block);
         return 0;
   }
   

    
   
   if (!strcasecmp(command, "status")) { printf("%s### FTM @ %s ####%s\n", KCYN, netaddress, KNRM); 
     if (verbose) {
        ebPeripheryStatus();
     }
   }
   

  
   //Cycle through selected CPUs
   uint8_t cpuIdx;
   for(cpuIdx = firstCpu; cpuIdx <= lastCpu; cpuIdx++)
   {
      //if the op is NOT fw load, verify that a valid fw is already in there
      if (strcasecmp(command, "loadfw")) {
         if(!(pAccess->pCores[cpuIdx].hasValidFW)) {
          fprintf(stderr, "CPU #%u NOT INITIALIZED!\nrun '%s %s -c  %u loadfw <firmware.bin>' to load fw to this core\n", cpuIdx, program, netaddress, cpuIdx);
          fprintf(stderr, "or  '%s %s -c -1 loadfw <firmware.bin>' to load fw to all core\n", program, netaddress);
          return -1;  
       } 
       
      
      /* -------------------------------------------------------------------- */
      if (!strcasecmp(command, "status")) {
         status(cpuIdx);
      }

      /* -------------------------------------------------------------------- */
      else if (!strcasecmp(command, "run")) {
       
         if (verbose) {
            printf("Starting FTM Core %u\n", cpuIdx);
         }
         ebstatus = eb_device_write(device, pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_START, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");

      }
     
      else if (!strcasecmp(command, "stop")) {
         if (verbose) {
            printf("Requesting FTM Core %u to stop\n", cpuIdx);
         }
         ebstatus = eb_device_write(device, pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_STOP_REQ, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");
      }

      else if (!strcasecmp(command, "idle")) {
       
         if (verbose) {
            printf("Setting BP of FTM COre %u to idle\n", cpuIdx);
         }
         ebstatus = eb_device_write(device, pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_IDLE, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");

      }

      else if (!strcasecmp(command, "fstop")) {
       
         if (verbose) {
            printf("Forcing FTM Core %u to stop\n", cpuIdx);
         }
         ebstatus = eb_device_write(device, pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_STOP_NOW, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");
      } 

      else if (!strcasecmp(command, "reset")) {
         if (verbose) {
            printf("Resetting FTM Core %u\n", cpuIdx);
         }
         ebstatus = eb_device_write(device, pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_RST, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 
      }
     
      else if (!strcasecmp(command, "preptime")) {
         if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(ebstatus, "failed to create cycle"); 
         eb_cycle_write(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_TPREP_OFFSET +0), EB_BIG_ENDIAN | EB_DATA32, (uint32_t)(uint64val>>32));
         eb_cycle_write(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_TPREP_OFFSET +4), EB_BIG_ENDIAN | EB_DATA32, (uint32_t)uint64val);
         if ((ebstatus = eb_cycle_close(cycle)) != EB_OK)  die(ebstatus, "failed to close write cycle");
    }
     
     else if (!strcasecmp(command, "setbp")) {
       int planIdx, planQty;
       eb_cycle_t cycle;
       uint32_t bp;
       
      if(!strcasecmp(bpstr, "idle")) planIdx = -1;
      else {planIdx = strtol(bpstr, 0, 10);}   
      
      //user gave us a planIdx. load corresponding ptr
      if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(ebstatus, "failed to create cycle"); 
      eb_cycle_read(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + pAccess->pCores[cpuIdx].actOffs + FTM_PAGE_PLANQTY_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      //if the user wanted to go to idle, read idle ptr from interface, else read plan ptr
      if(planIdx == -1)
      {eb_cycle_read(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_IDLE_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      else 
      {eb_cycle_read(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + pAccess->pCores[cpuIdx].actOffs + FTM_PAGE_PLANS_OFFSET + 4*planIdx), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      if((ebstatus = eb_cycle_close(cycle)) != EB_OK) die(ebstatus, "failed to close read cycle"); 
  
      planQty  = (uint32_t)tmpRead[0];
      bp       = (uint32_t)tmpRead[1];
      // Check and write to BP
      if(bp != FTM_NULL && planIdx < planQty) 
      {
         printf("Writing plan %d @ 0x%08x to BP\n", planIdx, bp);
         ebstatus = eb_device_write(device, (uint32_t)pAccess->pCores[cpuIdx].ramAdr + pAccess->pCores[cpuIdx].actOffs + FTM_PAGE_BP_OFFSET, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)bp, 0, eb_block);
         if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");  
      } else
      { 
         if (planIdx >= planQty) printf ("Sorry, but the plan index is neither idle nor 0 <= %d (planIdx) <  %u (planQty)\n", planIdx, planQty);
         else printf ("Found a NULL ptr at plan idx %d, something is wrong\n", planIdx);
      }
       
     }
    
    
    
    
    else if (!strcasecmp(command, "loadfw")) {
      printf("Putting CPU into Reset for FW load\n");
      ebstatus = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_SET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)resetBits, 0, eb_block);
      if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle");
      
      char cOffs[11] = "0x00000000";
      char cmdStr[80 + FILENAME_LEN];

      itoa(pAccess->pCores[cpuIdx].ramAdr, &cOffs[2], 16);
      strcpy (cmdStr, "eb-put -p -b ");
      strcat (cmdStr, netaddress);
      strcat (cmdStr, " ");
      strcat (cmdStr, cOffs);
      strcat (cmdStr, " ");
      strcat (cmdStr, filename);

      if (verbose) {
         printf("Loading %s to CPU %u @ %s\n", filename, cpuIdx, cOffs);  
      } else printf("Loading Firmware to CPU %u\n", cpuIdx);

      FILE* ebput = popen(cmdStr, "w");
      pclose(ebput);
            
      printf("Releasing all CPUs from Reset\n\n");
      ebstatus = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_CLR), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)resetBits, 0, eb_block);
      if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 
    
    }
     
     else if (!strcasecmp(command, "swap")) {
       
       if (verbose) {
         printf("Swapping Active/Inactive page on CPU %u\n", cpuIdx);
       }
       ebstatus = eb_device_write(device, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET), EB_BIG_ENDIAN | EB_DATA32, 
                                    (eb_data_t)CMD_COMMIT_PAGE, 0, eb_block);
      if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 

     } 
     
     else if (!strcasecmp(command, "condump")) {
       
       if (verbose) {
         printf("Commanding FTM CPU %u to show FTM Data on console\n", cpuIdx);
       }
       ebstatus = eb_device_write(device, pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET, EB_BIG_ENDIAN | EB_DATA32, CMD_SHOW_ACT, 0, eb_block) ;
       if (ebstatus != EB_OK) die(ebstatus, "failed to create cycle"); 
       sleep(1); 
     } 
     
     else if (!strcasecmp(command, "clear")) {
       
       if (verbose) {
         printf("Clearing all pages on FTM CPU %u \n", cpuIdx);
       }
       ebRamClear(pAccess->pCores[cpuIdx].ramAdr + pAccess->pCores[cpuIdx].inaOffs, BUF_SIZE);
       ebRamClear(pAccess->pCores[cpuIdx].ramAdr + pAccess->pCores[cpuIdx].actOffs, BUF_SIZE);
       sleep(1); 
     } 
     
     else if ((!strcasecmp(command, "put")) || (!strcasecmp(command, "get")) || (!strcasecmp(command, "dump"))) {
       
       targetOffset = pAccess->pCores[cpuIdx].inaOffs;
       
       if(!strcasecmp(command, "put")) {
          if(!readonly) {
            printf("Parsing %s ...", filename);
            pPage = parseXmlFile(filename);
            printf("done.\n");
            if(verbose) showFtmPage(pPage);
            serPage (pPage, pBufWrite, targetOffset, cpuIdx);
            ebRamWrite(pBufWrite, pAccess->pCores[cpuIdx].ramAdr + targetOffset, BUF_SIZE);
            printf("Writing %u bytes FTM Data to 0x%08x...", BUF_SIZE, pAccess->pCores[cpuIdx].ramAdr + targetOffset);
            printf("done.\n");
         } else fprintf(stderr, "No xml file specified\n");
         
      }
      
      if(!strcasecmp(command, "dump")) targetOffset = pAccess->pCores[cpuIdx].actOffs;
      ebRamRead(pAccess->pCores[cpuIdx].ramAdr + targetOffset, BUF_SIZE, pBufRead);
      printf("Reading %u bytes FTM Data from 0x%08x...", BUF_SIZE, pAccess->pCores[cpuIdx].ramAdr + targetOffset);
      printf("done.\n");
      
      //verify
      if(!readonly) {
         uint32_t i;
         for(i = 0; i<BUF_SIZE; i++) {
            if(!(pBufRead[i] == pBufWrite[i])) { 
               fprintf(stderr, "!ERROR! Verify of written ftmpage failed at offset 0x%08x\n", pAccess->pCores[cpuIdx].ramAdr + targetOffset +( i & ~0x3) );
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

    
   } 
      
}
   
  
   return 0;
}


