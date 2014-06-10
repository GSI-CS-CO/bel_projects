#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <etherbone.h>
#include "xmlaux.h"
#include "ftmx86.h"



#define MAX_DEVICES  100
#define BUF_SIZE     0x600
#define PACKET_SIZE  500

const char* program;
eb_device_t device;
eb_socket_t mySocket;

const    uint32_t devID_RAM 	      = 0x66cfeb52;
const    uint64_t vendID_CERN       = 0x000000000000ce42;
const    uint32_t devID_ClusterInfo = 0x10040086;
const    uint64_t vendID_GSI        = 0x0000000000000651;
char              devName_RAM_pre[] = "WB4-BlockRAM_";
		
volatile uint32_t embeddedOffset;



int ebRamOpen(const char* netaddress, uint8_t cpuId);
int ebRamRead(uint32_t address, uint32_t len, const uint8_t* buf);
int ebRamWrite(const uint8_t* buf, uint32_t address, uint32_t len);
int ebRamClose(void);


int ebRamOpen(const char* netaddress, uint8_t cpuId)
{
   
   eb_status_t status;
   int idx;
   int attempts;
   int num_devices;
   struct sdb_device devices[MAX_DEVICES];
   char              devName_RAM_post[4];
   uint8_t cpuQty;

   attempts   = 3;
   idx        = -1;

   
  
  /* open EB socket and device */
      if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &mySocket)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }
  if ((status = eb_device_open(mySocket, netaddress, EB_ADDR32 | EB_DATA32, attempts, &device)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }


  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_ClusterInfo, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: No lm32 clusterId rom found\n", program);
    ebRamClose();
    return 1;
  }

  if (num_devices > MAX_DEVICES) {
    fprintf(stderr, "%s: Way too many lm32 clusterId roms found, something's wrong\n", program);
    return 1;
  }

  if (idx > num_devices) {
    fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
    return 1;
  }

  if (idx == -1) {
    
    
  } else {
    
    
  }
  //printf("Found Cluster Info @ %08x\n", (uint32_t)devices[0].sdb_component.addr_first);
  //get number of CPUs and create search string
  eb_device_read(device, (uint32_t)devices[0].sdb_component.addr_first, EB_BIG_ENDIAN | EB_DATA32, &cpuQty, 0, eb_block);
  devName_RAM_post[0] = '0';
  devName_RAM_post[1] = '0' + (cpuQty & 0xf);
  devName_RAM_post[2] = '0' + (cpuId & 0xf);
  devName_RAM_post[3] = 0;
  
  if(cpuQty <= cpuId)
  {   
      fprintf(stderr, "The CpuId you gave me (%u) is higher than maximum (%u-1).\n", cpuId, cpuQty);
      ebRamClose();
      return 1;
  }
  
  
  printf("\tSearching for RAM of Cpu %u/%u\n", cpuId, (cpuQty & 0xf));
  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no RAM's found\n", program);
    ebRamClose();
    return 1;
  }

  if (num_devices > MAX_DEVICES) {
    fprintf(stderr, "%s: more devices found that tool supports (%d > %d)\n", program, num_devices, MAX_DEVICES);
    return 1;
  }

  if (idx > num_devices) {
    fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
    return 1;
  }

  if (idx == -1) {
    //printf("Found %u devs\n", num_devices);
    for (idx = 0; idx < num_devices; ++idx) {
      //printf("%.*s 0x%"PRIx64"\n", 19, &devices[idx].sdb_component.product.name[0], devices[idx].sdb_component.addr_first);
         if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0)
         {
            printf("\tfound %.*s @ 0x%08x\n", 19, &devices[idx].sdb_component.product.name[0], (uint32_t)devices[idx].sdb_component.addr_first);
            embeddedOffset = devices[idx].sdb_component.addr_first + FTM_SHARED_OFFSET;
            return 0;
         }
         
    }
  } else {
    printf("0x%"PRIx64"\n", devices[idx].sdb_component.addr_first);
  }
  
  fprintf(stderr, "Could not find RAM of CPU %u\n", cpuId);
  ebRamClose();
  return 1;
}

int ebRamClose()
{

   eb_status_t status;

      if ((status = eb_device_close(device)) != EB_OK) {
       fprintf(stderr, "%s: failed to close Etherbone device: %s\n", program, eb_status(status));
       return 1;
     }

     if ((status = eb_socket_close(mySocket)) != EB_OK) {
       fprintf(stderr, "%s: failed to close Etherbone socket: %s\n", program, eb_status(status));
       return 1;
     }
     
     return 0;
  
}

int ebRamRead(uint32_t address, uint32_t len, const uint8_t* buf)
{
   
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen;
   uint32_t* readin = (uint32_t*)buf;	

   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
            fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
            return 1;
         }  
      for(i=(j*partLen)>>2; i<(((j+1)*partLen)>>2);i++)  
      {
         
         eb_cycle_read(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32,  &readin[i]); 
         
      }
      eb_cycle_close(cycle);
   }   
   
   return len;
}

int ebRamWrite(const uint8_t* buf, uint32_t address, uint32_t len)
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen;
   uint32_t* writeout = (uint32_t*)buf;	
   
   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
            fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
            return 1;
         } 
      
      for(i=(j*partLen)>>2; i<(((j+1)*partLen)>>2);i++) 
      {
          
        eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, writeout[i]); 
      
      }
      eb_cycle_close(cycle);
   }
   return len;
}

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [command]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -x <filename>             select xml file with FTM data to copy to ftm cpu\n");
  fprintf(stderr, "  -c <cpu-id>               select a cpu by index#\n");
  fprintf(stderr, "  -v                        verbose operation: print ftm data\n");
  fprintf(stderr, "  -h                        display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  status                    report this ftm cpu's status\n");
  fprintf(stderr, "  run                       run this ftm cpu\n");
  fprintf(stderr, "  stop                      request stop on this ftm cpu\n");
  fprintf(stderr, "  fstop                     force stop on this ftm cpu\n");
  fprintf(stderr, "  put                       puts ftm data from xml file to cpu, then reads back ftm data from cpu.\n");
  fprintf(stderr, "  get                       gets ftm data from cpu.\n");
  fprintf(stderr, "\n");
}

static void die(eb_status_t status, const char* what) {
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  exit(1);
}



int main(int argc, char** argv) {

   uint32_t i, wSize;
   int opt;
   char *value_end;
   
   t_ftmPage* pPage = NULL;
   t_ftmPage* pNewPage = NULL;
   const char* netaddress, *command;
   uint8_t bufWrite[BUF_SIZE];
   uint8_t* pBufWrite;

   uint8_t bufRead[BUF_SIZE];
   uint8_t* pBufRead = &bufRead[0];
   uint8_t cpuId;
   char filename[64];
   uint8_t error, verbose, readonly;
   FILE *f;    
  
   cpuId     = 0;
   error     = 0;
   verbose   = 0;
   readonly  = 1;
   program = argv[0];
   while ((opt = getopt(argc, argv, "x:c:vh")) != -1) {
      switch (opt) {
         case 'x':  i=0;
                  while(i<64 && (optarg[i] != '\0')) { filename[i] = optarg[i]; i++;}
                  filename[i] = '\0';
                  readonly = 0;
                  break;
         case 'c':
            cpuId = strtol(optarg, &value_end, 0);
            if (*value_end || cpuId < 0 ||cpuId > 32) {
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
   
   printf("\nOpening CPU %u on %s ...\n", cpuId, netaddress);
   if(ebRamOpen(netaddress, cpuId)) return 1;
   printf("done.\n");

   if (optind+1 < argc) {
   command = argv[optind+1];
   } else {
   command = "status";
   }
   
   /* -------------------------------------------------------------------- */
  if (!strcasecmp(command, "status")) {
    
  }

  /* -------------------------------------------------------------------- */
  else if (!strcasecmp(command, "run")) {
    
    if (verbose) {
      printf("Starting CPU %u\n", cpuId);
    }
    
  }
  
  else if (!strcasecmp(command, "stop")) {
    
    if (verbose) {
      printf("Requesting CPU %u to stop\n", cpuId);
    }
    
  }
  
  else if (!strcasecmp(command, "fstop")) {
    
    if (verbose) {
      printf("Forcing CPU %u to stop\n", cpuId);
    }
    
  } 
  
  else if ((!strcasecmp(command, "put")) || (!strcasecmp(command, "get"))) {
    if(!strcasecmp(command, "put")) {
       if(!readonly) {
         printf("Parsing %s ...", filename);
         pPage = parseXml(filename);
         printf("done.\n");
         if(verbose) showFtmPage(pPage);
         pBufWrite = serPage (pPage, &bufWrite[0], FTM_SHARED_OFFSET);
         printf("Writing %u bytes FTM Data ...", ebRamWrite(&bufWrite[0], embeddedOffset, BUF_SIZE));
         printf("done.\n");
      } else fprintf(stderr, "No xml file specified\n");
   }

   printf("Reading %u bytes FTM Data ...", ebRamRead(embeddedOffset, BUF_SIZE, pBufRead));
   printf("done.\n");
   
   pNewPage = deserPage(calloc(1, sizeof(t_ftmPage)), &bufRead[0], FTM_SHARED_OFFSET);
   if(pNewPage != NULL)
   {  
      printf("Deserialization successful.\n\n");
      if(verbose || (!strcasecmp(command, "get")) ) showFtmPage(pNewPage);
   }   
   else printf("Deserialization FAILED! Corrupt/No Data ?\n");
   }   
    
  
   

  

   
	   
	ebRamClose();   
   return 0;
}


