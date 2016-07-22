#include "access.h"
#include "hwregs.h"
#include "ftmx86.h"
#include "xmlaux.h"
#include "fancy.h"
#include <time.h>

uint32_t ftm_shared_offs;
t_ftmAccess* p;
t_ftmAccess ftmAccess;
eb_device_t device;
eb_socket_t mySocket;

/// Expected Firmware Version ///
//const char myVer[] = "0.2.1\n";
//deprecated - using EXP_VER symbol from Makefile instead

const char expName[] = "ftm\n";
////////////////////////////////

const uint64_t vendID_CERN       = 0x000000000000ce42;
const uint64_t vendID_GSI        = 0x0000000000000651;

const uint32_t devID_SysCon      = 0xff07fc47;
const uint32_t devID_PPS         = 0xde0d8ced;

const uint32_t devID_Reset       = 0x3a362063;
const uint32_t devID_RAM         = 0x66cfeb52;
const uint32_t devID_CoreRAM     = 0x54111351;
const uint32_t devID_SharedRAM   = 0x81111444;

const uint32_t devID_ClusterInfo = 0x10040086;
const uint32_t devID_ClusterCB   = 0x10041000;
const uint32_t devID_Ebm         = 0x00000815;

const uint32_t devID_ECA         = 0xb2afc251;
const uint32_t eva_time_hi_get   = 0x18;
const uint32_t eva_time_low_get  = 0x1c;

const char     devName_RAM_pre[] = "WB4-BlockRAM_";

extern uint8_t show_time;
uint32_t currentTimeHigh;
uint32_t currentTimeLow;
  
static int die(eb_status_t status, const char* what)
{  
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  return -1;
}


static void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}


static int ftmRamRead(uint32_t address, const uint8_t* buf, uint32_t len, uint32_t bufEndian)
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
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle");
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         //printf("%4u %08x -> %p\n", i, (address+(i<<2)), &readin[i]);
         eb_cycle_read(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, &tmpReadin[i]);
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");
      for(i= start>>2; i< (start + partLen) >>2;i++) {
        
        if (bufEndian == LITTLE_ENDIAN)  readin[i] = SWAP_4((uint32_t)tmpReadin[i]);
        else                             readin[i] = (uint32_t)tmpReadin[i];
        
      } //this is important caus eb_data_t is 64b wide!
      start = start + partLen;
   }
      
   return 0;
}

static int ftmRamWrite(uint32_t address, const uint8_t* buf, uint32_t len, uint32_t bufEndian)
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start, data;
   uint32_t* writeout = (uint32_t*)buf;   
   
   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         if (bufEndian == LITTLE_ENDIAN)  data = SWAP_4(writeout[i]);
         else                             data = writeout[i];
         
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)data); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close write cycle");
      start = start + partLen;
   }
   
   return 0;
}

static int ftmRamClear(uint32_t address, uint32_t len)
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start;  
   
   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, 0); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  return die(status, "failed to close write cycle");
      start = start + partLen;
   }
   return 0;
}


static uint8_t isFwValid(struct  sdb_device* ram, int cpuIdx, const char* sVerExp, const char* sName)
{
  uint8_t validity = 1;
  uint32_t len = FWID_LEN/4;
  eb_data_t fwdata[FWID_LEN/4];
  char cBuff[FWID_LEN];
  char* pos;
  
  eb_cycle_t cycle;
  eb_status_t status;
  
  uint8_t verExpMaj, verExpMin, verExpRev;
  uint8_t verFndMaj=0, verFndMin=0, verFndRev=0;
  uint32_t verExp, verFnd;
  verExpMaj = sVerExp[0] - '0';
  verExpMin = sVerExp[2] - '0';
  verExpRev = sVerExp[4] - '0';
  
  uint32_t i, j;
  //RAM Big enough to actually contain a FW ID?
  if ((ram->sdb_component.addr_last - ram->sdb_component.addr_first + 1) >= (FWID_LEN + BOOTL_LEN)) {
    if ((status = eb_cycle_open(device, 0, 0, &cycle)) != EB_OK)
      die(status, "eb_cycle_open");
    for (j = 0; j < len; ++j)
      eb_cycle_read(cycle, ram->sdb_component.addr_first + BOOTL_LEN + j*4, EB_DATA32|EB_BIG_ENDIAN, &fwdata[j]);
    if ((status = eb_cycle_close(cycle)) != EB_OK)
      die(status, "eb_cycle_close");

   for (j = 0; j < len; ++j) {
      for (i = 0; i < 4; i++) {
        cBuff[j*4+i] = (char)(fwdata[j] >> (8*(3-i)) & 0xff);
      }  
    }
   
  //check for magic word
  if(strncmp(cBuff, "UserLM32", 8)) {validity = 0;} 
  if(!validity) { printf("Core #%02u: No firmware found!\n", cpuIdx); return 0; }

  //check project
  pos = strstr(cBuff, "Project     : ");
  if(pos != NULL) {
    pos += 14;
    if(strncmp(pos, sName, strlen(sName))) {validity = 0;} 
  } else { printf("Core #%02u: This is no ftm firmware, name does not match!\n", cpuIdx); return 0;}
  
  //check version
  pos = strstr(cBuff, "Version     : ");
  if(pos != NULL) {
    pos += 14;
    verFndMaj = pos[0] - '0';
    verFndMin = pos[2] - '0';
    verFndRev = pos[4] - '0';
  } else {validity = 0;}
  } else {validity = 0;}
  
  verExp = (verExpMaj *100 + verExpMin *10 + verExpRev);
  verFnd = (verFndMaj *100 + verFndMin *10 + verFndRev);
  
  if(verExp > verFnd ) {
    validity = 0;
    printf("Core #%02u: Expected firmware %u.%u.%u, but found only %u.%u.%u! If you are sure, use -o to override.\n", cpuIdx, verExpMaj, verExpMin, verExpRev, verFndMaj, verFndMin, verFndRev);
    return 0;  
  }
  if(verExp < verFnd ) {
    printf("Core #%02u: Expected firmware %u.%u.%u is lower than found %u.%u.%u. If you are sure, use -o to override.\n", cpuIdx, verExpMaj, verExpMin, verExpRev, verFndMaj, verFndMin, verFndRev);
    return 0;
  }
  
  //no fwid found. try legacy  
  return validity;
    
}

static int ftmPut(uint32_t dstCpus, t_ftmPage*  pPage, uint8_t* bufWrite, uint32_t len) {
  uint32_t baseAddr, offs, cpuIdx, i; 
  uint8_t* bufRead = (uint8_t *)malloc(len);
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = p->pCores[cpuIdx].ramAdr;
      offs      = p->pCores[cpuIdx].inaOffs;

      printf("InaOffs: 0x%08x\n", offs);

      memset(bufWrite, 0, len);
      if(serPage (pPage, bufWrite, offs, cpuIdx) == NULL) return -1;
      ftmRamWrite(baseAddr + (offs & p->pCores[cpuIdx].mask), bufWrite, len, BIG_ENDIAN);
      printf("Wrote %u byte schedule to CPU %u at 0x%08x.", len, cpuIdx, baseAddr + (offs & p->pCores[cpuIdx].mask));
      printf("Verify..."); 
      ftmRamRead(baseAddr + (offs & p->pCores[cpuIdx].mask), bufRead, len, BIG_ENDIAN);
      for(i = 0; i<len; i++) {
        if(!(bufRead[i] == bufWrite[i])) { 
          fprintf(stderr, "!ERROR! \nVerify failed for CPU %u at offset 0x%08x\n", cpuIdx, baseAddr + (offs & p->pCores[cpuIdx].mask) +( i & ~0x3) );
          free(bufRead);
          return -2;
        }
      }
      printf("OK\n");
    }
  }
  free(bufRead);
  printf("done.\n");
  return 0;  

}

int ftmOpen(const char* netaddress, uint8_t overrideFWcheck)
{
  eb_cycle_t cycle;
  eb_status_t status;
  int cpuIdx, idx;
  int attempts;
  int num_devices;
  struct sdb_device devices[MAX_DEVICES];
  char              devName_RAM_post[4];
  struct sdb_bridge CluCB;
  
  uint32_t validCpus = 0;
  uint32_t masks[32];
  
  eb_data_t tmpRead[4];
  
  attempts   = 3;
  idx        = -1;
  p = (t_ftmAccess* )&ftmAccess;


  /* open EB socket and device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &mySocket))               != EB_OK) {die(status, "failed to open Etherbone socket"); return 0;}
  if ((status = eb_device_open(mySocket, netaddress, EB_ADDR32 | EB_DATA32, attempts, &device)) != EB_OK) {die(status, "failed to open Etherbone device"); return 0;}

  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_ClusterCB, (struct sdb_device*)&CluCB, &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  p->clusterAdr = CluCB.sdb_component.addr_first;
  
  //find reset ctrl
  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_Reset, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no reset controller found\n", program);
    goto error;
  }
  p->resetAdr = (eb_address_t)devices[0].sdb_component.addr_first;

  //get wr-syscon
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_SysCon, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  if (num_devices == 0) {
    fprintf(stderr, "%s: No wr syscon found\n", program);
    goto error;
  }
  p->sysConAdr = (eb_address_t)devices[0].sdb_component.addr_first;
  
  //get pps
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_PPS, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  if (num_devices == 0) {
    fprintf(stderr, "%s: No wr syscon found\n", program);
    goto error;
  }
  p->ppsAdr = (eb_address_t)devices[0].sdb_component.addr_first;


  //get clusterInfo
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_GSI, devID_ClusterInfo, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  if (num_devices == 0) {
    fprintf(stderr, "%s: No lm32 clusterId rom found\n", program);
    goto error;
  }
  //get number of CPUs
  num_devices = MAX_DEVICES;
  status = eb_device_read(device, (eb_address_t)devices[0].sdb_component.addr_first, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0], 0, eb_block);
  if (status != EB_OK) {return die(status, "failed to create cycle");}
  p->cpuQty = (uint8_t)tmpRead[0];
  //FIXME Adapt Cluster Info correctly and do a read here!!!
  p->thrQty = 8;
  p->pCores =  malloc(p->cpuQty * sizeof(t_core));
/*
  // Get Shared RAM
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_GSI, devID_SharedRAM, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for Shared RAM ");}
  //Old or new Gateware ?
  //FIXME: the cumbersome legacy code has to go sometime
  if(num_devices < 1) {
    //Old
    if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
    {return die(status, "failed to when searching for Shared RAM ");}
  }
  p->sharedAdr = (eb_address_t)devices[0].sdb_component.addr_first;
*/
  // Get prioq
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity(device, PRIO_SDB_VENDOR_ID, PRIO_SDB_DEVICE_ID, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for Priority Queue ");}
  if (num_devices == 0) {
    fprintf(stderr, "%s: No Priority Queue found\n", program);
    goto error;
  }
  p->prioQAdr = (eb_address_t)devices[0].sdb_component.addr_first;

  
  // Get EBM
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_Ebm, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  if (num_devices == 0) {
    fprintf(stderr, "%s: No Etherbone Master found\n", program);
    goto error;
  }
  p->ebmAdr = (eb_address_t)devices[0].sdb_component.addr_first;
  
  //Get RAMs 
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_GSI, devID_CoreRAM, &devices[0], &num_devices)) != EB_OK)
  {return die(status, "failed to when searching for device");}
  
  //Old or new Gateware ?
  //FIXME: the cumbersome legacy code has to go sometime
  //if(overrideFWcheck) printf("Gate-/firmware check disabled by override option\n");
  
  if(num_devices > 0) {
    //new
    ftm_shared_offs = FTM_SHARED_OFFSET_NEW;
    for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      p->pCores[cpuIdx].ramAdr = devices[cpuIdx].sdb_component.addr_first;
      p->pCores[cpuIdx].mask = devices[cpuIdx].sdb_component.addr_last - devices[cpuIdx].sdb_component.addr_first;
      //check for valid firmware
      uint8_t isValid = 0;
      if(overrideFWcheck) isValid = 1;
      else                { isValid = isFwValid(&devices[cpuIdx], cpuIdx, &EXP_VER[0], &expName[0]);}
      validCpus |= (isValid << cpuIdx);
      p->pCores[cpuIdx].hasValidFW = isValid;
    }
  } else {
    //Old
    ftm_shared_offs = FTM_SHARED_OFFSET_OLD;
    if(!overrideFWcheck) {
      printf("ERROR: FTM is using old gateware. Sure this is the FTM you want ? Use option '-o' if you want to override\n");
      goto error;
    }
    
    for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      devName_RAM_post[0] = '0';
      devName_RAM_post[1] = '0' + (p->cpuQty & 0xf);
      devName_RAM_post[2] = '0' + (cpuIdx  & 0xf);
      devName_RAM_post[3] =  0;

      num_devices = MAX_DEVICES;
      if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
      {return die(status, "failed to when searching for device");}
      
      for (idx = 0; idx < num_devices; ++idx) {
        if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0) {
          p->pCores[cpuIdx].ramAdr = devices[idx].sdb_component.addr_first;
          p->pCores[cpuIdx].hasValidFW = 1;
          validCpus |= (1 << cpuIdx);
        }
      }
    }
  } 
    
  // get the active, inactive and shared pointer values from the core RAM
  for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
    if (p->pCores[cpuIdx].hasValidFW) {
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
      eb_cycle_read(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_PACT_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      eb_cycle_read(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_PINA_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
      eb_cycle_read(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_SHARED_PTR_OFFSET),   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[2]); 
      
      if ((status = eb_cycle_close(cycle)) != EB_OK) die(status, "failed to close read cycle");


      p->pCores[cpuIdx].actOffs     = (uint32_t) tmpRead[0];
      p->pCores[cpuIdx].inaOffs     = (uint32_t) tmpRead[1];
      p->pCores[cpuIdx].sharedOffs  = (uint32_t) tmpRead[2];

      //printf("Act: 0x%08x\n", p->pCores[cpuIdx].actOffs);
    
    } else printf("Core #%u: Can't read schedule data offsets - no valid firmware present.\n", cpuIdx);
  }
  p->validCpus = validCpus;
  return validCpus;

  error:
  p->validCpus = 0;
  ftmClose();
  return -2; //dummy
}

int ftmClose(void)
{
  eb_status_t status;
  if(p->pCores != NULL) free(p->pCores);
  if ((status = eb_device_close(device))   != EB_OK) return die(status, "failed to close Etherbone device");
  if ((status = eb_socket_close(mySocket)) != EB_OK) return die(status, "failed to close Etherbone socket");
  return 0;
}

int ftmRst(void) {
  eb_status_t status;
    status = eb_device_write(device, (eb_address_t)(p->resetAdr + FTM_RST_FPGA), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)FTM_RST_FPGA_CMD, 0, eb_block);
    if (status != EB_OK) return die(status, "failed to create cycle"); 
    return 0;
}

int ftmCpuRst(uint32_t dstCpus) {

  eb_status_t status;
  
  printf("Resetting CPU(s)...\n");
  status = eb_device_write(device, (eb_address_t)(p->resetAdr + FTM_RST_SET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpus, 0, eb_block);
  if (status != EB_OK) return die(status, "failed to create cycle");
  status = eb_device_write(device, (eb_address_t)(p->resetAdr + FTM_RST_CLR), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpus, 0, eb_block);
  if (status != EB_OK) return die(status, "failed to create cycle"); 
  printf("Done.\n\n");
  return 0;
}

int ftmFwLoad(uint32_t dstCpus, const char* filename) {
  

  FILE *file;
  uint8_t* buffer;
  unsigned long fileLen;
  eb_status_t status;
  
  uint32_t cpuIdx;
  
  //Open file
  file = fopen(filename, "rb");
  if (!file)
  {
    fprintf(stderr, "Unable to open file %s", filename);
    return -3;
  }

  //Get file length
  fseek(file, 0, SEEK_END);
  fileLen=ftell(file);
  fseek(file, 0, SEEK_SET);
  //Allocate memory
  buffer=(uint8_t *)malloc(fileLen+1);
  if (!buffer)
  {
    fprintf(stderr, "Memory error!");
    fclose(file);
    return -2;
  }
  //Read file contents into buffer
  fileLen = fread(buffer, 1, fileLen, file);
  fclose(file);
  
  printf("Putting CPU(s) into Reset for FW load\n");
  status = eb_device_write(device, (eb_address_t)(p->resetAdr + FTM_RST_SET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpus, 0, eb_block);
  if (status != EB_OK) return die(status, "failed to create cycle");
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      //Load FW
      printf("Loading %s to CPU %u @ 0x%08x\n", filename, cpuIdx, p->pCores[cpuIdx].ramAdr);  
      ftmRamWrite(p->pCores[cpuIdx].ramAdr, buffer, fileLen, LITTLE_ENDIAN);
    }
  }
  free(buffer);

  printf("Releasing CPU(s) from Reset\n\n");
  status = eb_device_write(device, (eb_address_t)(p->resetAdr + FTM_RST_CLR), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpus, 0, eb_block);
  if (status != EB_OK) return die(status, "failed to create cycle"); 

  printf("Done.\n");
  return 0;

}


int ftmThrRst(uint64_t dstBitField) {
  return 0;
}


int v02FtmCommand(uint32_t dstCpus, uint32_t command) {

  eb_status_t status;
  eb_cycle_t cycle;
  uint32_t cpuIdx;
  
  if(dstCpus) {
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle");
  }
  else return 0;
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      printf("cmd adr: 0x%08x\n", (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET));  
      eb_cycle_write(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)command);
    }
  }
  
  if ((status = eb_cycle_close(cycle)) != EB_OK) return  die(status, "failed to close write cycle");
  return 0;
}

int ftmSignal(uint64_t dstThr, uint32_t offset, uint64_t value, uint64_t mask) {
  
  eb_status_t status;
  eb_cycle_t cycle;
  uint32_t thrIdx, cpuIdx, i;
  uint64_t val, read;
  eb_data_t tmpRead[2];
  eb_address_t addr;
  
  //go through all marked threads
  for(i=0;i < (p->thrQty * p->thrQty);i++) {
    if((dstThr >> i) & 0x1) {
      //get thread shared com RAM address
      cpuIdx = i / p->thrQty;
      thrIdx = i % p->thrQty;
      addr = (eb_address_t)(p->sharedAdr + cpuIdx*CPU_SHARED_SIZE + thrIdx*THR_SHARED_SIZE + offset);
      printf("Signal address: 0x%08x ", (uint32_t)addr);
      //read current value
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle");
      eb_cycle_read(cycle, addr + 0, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      eb_cycle_read(cycle, addr + 4, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close write cycle");
 
      printf("CurrentValue: 0x%08x%08x ", (uint32_t)tmpRead[1], (uint32_t)tmpRead[0]);
      //modify
      read = (uint64_t)(uint32_t)tmpRead[0] | ((uint64_t)tmpRead[1])<<32;
      val = (read & ~mask) | (value & mask);
      printf("NewValue: 0x%08x%08x\n", (uint32_t)(val>>32), (uint32_t)val);
      //write back 
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle");
      if ((uint32_t)mask)         eb_cycle_write(cycle, addr + 0, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)val));
      if ((uint32_t)(mask >> 32)) eb_cycle_write(cycle, addr + 4, EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)(val>>32)));
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close write cycle");
      
    }
  }
  return 0;
}


int v02FtmCheckString(const char* sXml) {

  uint8_t     buff[65536];
  int         ret;
  t_ftmPage*  pPage = parseXmlString(sXml);
  
  
  if(serPage (pPage, (uint8_t*)&buff[0], 0, 0) != NULL) {
    printf("Schedule OK\n");
    ret = 0;
  } else {
    fprintf(stderr, "There were errors in the Schedule.\n");
    ret = -1;
  }
  
  if(pPage != NULL) free(pPage);

  return ret;
}

int v02FtmPutString(uint32_t dstCpus, const char* sXml) {
  uint8_t    bufWrite[BUF_SIZE];
  t_ftmPage*  pPage;
  pPage = parseXmlString(sXml);
    
  return ftmPut(dstCpus, pPage, (uint8_t*)&bufWrite, BUF_SIZE);
}
  
int v02FtmPutFile(uint32_t dstCpus, const char* filename) {
  uint8_t    bufWrite[BUF_SIZE];
  t_ftmPage*  pPage = parseXmlFile(filename);
  
  return ftmPut(dstCpus, pPage, (uint8_t*)&bufWrite, BUF_SIZE);
  
}



int v02FtmDump(uint32_t srcCpus, uint32_t len, uint8_t actIna, char* stringBuf, uint32_t lenStringBuf) {
  uint32_t baseAddr, offs, cpuIdx;  
  t_ftmPage* pPage;
  uint8_t* bufRead = (uint8_t *)malloc(len);
  char* bufStr = stringBuf;
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((srcCpus >> cpuIdx) & 0x1) {
      baseAddr  = p->pCores[cpuIdx].ramAdr;
      if(actIna == ACTIVE)  offs = p->pCores[cpuIdx].actOffs; 
      else              offs = p->pCores[cpuIdx].inaOffs;
      
      ftmRamRead( baseAddr + (offs & p->pCores[cpuIdx].mask), bufRead, len, BIG_ENDIAN);
      pPage = deserPage(calloc(1, sizeof(t_ftmPage)), bufRead, offs);
      if(pPage != NULL) {  
         printf("Deserialization successful.\n\n");
         if (lenStringBuf - (bufStr - stringBuf) < 2048) {printf("String buffer running too low, aborting.\n"); return (uint32_t)(bufStr - stringBuf);}
         SNTPRINTF(bufStr, "---CPU %u %s page---\n", cpuIdx, "active"); //don't do zero termination in between dumps
         bufStr += showFtmPage(pPage, bufStr); //don't do zero termination in between dumps
      } else {printf("Deserialization for CPU %u FAILED! Corrupt/No Data ?\n", cpuIdx); return -1;}
    }
  }
  *bufStr++ = 0x00; // zero terminate all dumps
  return (uint32_t)(bufStr - stringBuf); // return number of characters
}

int v02FtmClear(uint32_t dstCpus) {
  uint32_t baseAddr, offs, cpuIdx; 
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = p->pCores[cpuIdx].ramAdr;
      offs      = p->pCores[cpuIdx].inaOffs;
      ftmRamClear(baseAddr + (offs & p->pCores[cpuIdx].mask), BUF_SIZE);
      printf("Cleared %u bytes in inactive page of CPU %u at 0x%08x.", BUF_SIZE, cpuIdx, baseAddr + (offs & p->pCores[cpuIdx].mask));
    }  
  }
  printf("done.\n");
  return 0;  

}

int ftmSetPreptime(uint32_t dstCpus, uint64_t tprep) {
  eb_cycle_t cycle;
  eb_status_t status;
  uint32_t cpuIdx; 
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      eb_cycle_write(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_TPREP_OFFSET +0), EB_BIG_ENDIAN | EB_DATA32, (uint32_t)(tprep>>35));
      eb_cycle_write(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_TPREP_OFFSET +4), EB_BIG_ENDIAN | EB_DATA32, (uint32_t)(tprep));
      if ((status = eb_cycle_close(cycle)) != EB_OK)  return die(status, "failed to close write cycle");
    }
  }
  return 0;    
}

int ftmSetDuetime(uint64_t tdue) {
  eb_cycle_t cycle;
  eb_status_t status;
  
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
  eb_cycle_write(cycle, (eb_address_t)(p->prioQAdr + PRIO_TX_MAX_WAIT_RW), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)tdue));
  if ((status = eb_cycle_close(cycle)) != EB_OK)  return die(status, "failed to close write cycle");
  return 0;    
}

int ftmSetTrntime(uint64_t ttrn) {

  //FIXME This function is obsolete!!!
  return 0;    
}

int ftmSetMaxMsgs(uint64_t maxmsg) {
  eb_status_t status;
  
  if ((status = eb_device_write(device, (eb_address_t)(p->prioQAdr + PRIO_TX_MAX_MSGS_RW), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)maxmsg), 0, eb_block))  != EB_OK) 
    return die(status, "failed to create cycle"); 
  return 0;    
}

int v02FtmSetBp(uint32_t dstCpus, int32_t planIdx) {

  int planQty;
  eb_cycle_t cycle;
  eb_status_t status;
  uint32_t bp;
  eb_data_t tmpRead[3];
  uint32_t baseAddr, offs, cpuIdx; 
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = p->pCores[cpuIdx].ramAdr;
      offs      = p->pCores[cpuIdx].actOffs;
      
      //user gave us a planIdx. load corresponding ptr
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      eb_cycle_read(cycle, (eb_address_t)(baseAddr + (offs & p->pCores[cpuIdx].mask) + FTM_PAGE_PLANQTY_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      //if the user wanted to go to idle, read idle ptr from interface, else read plan ptr
      if(planIdx == -1)
      {eb_cycle_read(cycle, (eb_address_t)(baseAddr + ftm_shared_offs + FTM_IDLE_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      else 
      {eb_cycle_read(cycle, (eb_address_t)(baseAddr + (offs & p->pCores[cpuIdx].mask) + FTM_PAGE_PLANS_OFFSET + 4*planIdx), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      if((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle"); 

      planQty  = (uint32_t)tmpRead[0];
      bp       = (uint32_t)tmpRead[1];
      // Check and write to BP
      if(bp != FTM_NULL && planIdx < planQty) {
        printf("Writing plan %d @ 0x%08x to BP\n", planIdx, bp);
        status = eb_device_write(device, (eb_address_t)(baseAddr + (offs & p->pCores[cpuIdx].mask) + FTM_PAGE_BP_OFFSET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)bp, 0, eb_block);
        if (status != EB_OK) return die(status, "failed to create cycle"); 
        
      } else { 
        if (planIdx >= planQty) printf ("Sorry, but the plan index is neither idle nor 0 <= %d (planIdx) <  %u (planQty)\n", planIdx, planQty);
        else printf ("Found a NULL ptr at plan idx %d, something is wrong\n", planIdx);
        return -1;
      }
    }  
  }
  return 0; 
}



int v02FtmFetchStatus(uint32_t* buff, uint32_t len) {
  uint32_t cpuIdx, thrIdx, offset;
  uint32_t coreStateSize = (CPU_STATE_SIZE + p->thrQty * THR_STATE_SIZE);
  uint32_t returnedLen = ((EBM_SEMA_RW + PRIO_TX_MAX_WAIT_RW)>>2) + WR_STATE_SIZE + p->cpuQty * coreStateSize;
  eb_data_t time1, time0, time2;
  eb_address_t eca_base;
  int eca_num_devices;
  struct sdb_device eca_devices[1];
    
  if (len < returnedLen) return (len - returnedLen);
  
  eb_address_t tmpAdr;
  eb_data_t tmpRead[6];
  eb_cycle_t cycle;
  eb_status_t status;
  
  offset = 0;

  // read EBM status
  ftmRamRead(p->ebmAdr + EBM_STATUS_GET, (const uint8_t*)&buff[EBM_STATUS_GET>>2], EBM_SEMA_RW, BIG_ENDIAN);

  offset += (EBM_SEMA_RW)>>2; //advance offset

  // read PrioQ status 
  ftmRamRead(p->prioQAdr + PRIO_MODE_GET,     (const uint8_t*)&buff[(EBM_SEMA_RW + PRIO_MODE_GET)>>2],    4,                                            BIG_ENDIAN);
  ftmRamRead(p->prioQAdr + PRIO_ST_FULL_GET,  (const uint8_t*)&buff[(EBM_SEMA_RW + PRIO_ST_FULL_GET)>>2], PRIO_CNT_OUT_ALL_GET_1 - PRIO_ST_FULL_GET +4, BIG_ENDIAN);
  offset += (PRIO_CNT_OUT_ALL_GET_1 + 4)>>2; //advance offset

  //read WR State
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
  eb_cycle_read(cycle, (eb_address_t)p->ppsAdr + PPS_STATE,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
  
  eb_cycle_read(cycle, (eb_address_t)p->ppsAdr + PPS_CNTR_UTCLO,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[2]);
  eb_cycle_read(cycle, (eb_address_t)p->ppsAdr + PPS_CNTR_UTCHI,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");
  
  buff[offset + WR_STATUS] = (uint32_t)tmpRead[0] & 0x6;
  buff[offset + WR_UTC_HI] = (uint32_t)tmpRead[1] & 0xff;
  buff[offset + WR_UTC_LO] = (uint32_t)tmpRead[2];
  
  // try to get the white rabbit time
  if (show_time)
  {
    // search for ECA version 2+
    eca_num_devices = sizeof(eca_devices)/sizeof(eca_devices[0]);
    if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_ECA, &eca_devices[0], &eca_num_devices)) == EB_OK)
    {
      // setup ECA base address
      eca_base = (eb_address_t)eca_devices[0].sdb_component.addr_first;
      // get current time
      do
      {
        if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
        eb_cycle_read(cycle, eca_base + eva_time_hi_get,  EB_DATA32, &time1);
        eb_cycle_read(cycle, eca_base + eva_time_low_get, EB_DATA32, &time0);
        eb_cycle_read(cycle, eca_base + eva_time_hi_get,  EB_DATA32, &time2);
        if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");
      } while (time1 != time2);
      currentTimeHigh = time1;
      currentTimeLow = time0;
    }
    else
    {
      return die(status, "ECA not found");
    }
  }

  offset += WR_STATE_SIZE; 

  // read CPU status'
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((p->validCpus >> cpuIdx) & 0x1) {
      tmpAdr = p->pCores[cpuIdx].ramAdr + ftm_shared_offs;


      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_STAT_OFFSET,        EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_SHARED_PTR_OFFSET,  EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]); 
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET,       EB_BIG_ENDIAN | EB_DATA32, &tmpRead[2]);
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET+4,     EB_BIG_ENDIAN | EB_DATA32, &tmpRead[3]);
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_PACT_OFFSET,        EB_BIG_ENDIAN | EB_DATA32, &tmpRead[4]);
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_PINA_OFFSET,        EB_BIG_ENDIAN | EB_DATA32, &tmpRead[5]);
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");

      //Future work: change everything to new register layout in v03
      buff[offset + cpuIdx*coreStateSize  + CPU_STATUS]   = (uint32_t)tmpRead[0] & 0xffff;
      buff[offset + cpuIdx*coreStateSize  + CPU_MSGS]     = (uint32_t)tmpRead[0] >> 16;
      buff[offset + cpuIdx*coreStateSize  + CPU_SHARED]   = (uint32_t)tmpRead[1];
      buff[offset + cpuIdx*coreStateSize  + CPU_TPREP_HI] = (uint32_t)tmpRead[2];
      buff[offset + cpuIdx*coreStateSize  + CPU_TPREP_LO] = (uint32_t)tmpRead[3];
    
//TODO
      //Future work: change everything to new register layout in v03. And include real threads !!!
      uint32_t tmp = buff[offset + cpuIdx*coreStateSize  + CPU_STATUS];
      if(tmp & STAT_RUNNING) buff[offset + cpuIdx*coreStateSize  + CPU_THR_RUNNING] = 1;
      if(tmp & STAT_WAIT) buff[offset + cpuIdx*coreStateSize  + CPU_THR_WAITING] = 1;
      if(tmp & STAT_IDLE)    buff[offset + cpuIdx*coreStateSize  + CPU_THR_IDLE]    = 1;
      if(tmp & STAT_ERROR)   buff[offset + cpuIdx*coreStateSize  + CPU_THR_ERROR]   = 1;

      p->pCores[cpuIdx].actOffs     = (uint32_t) tmpRead[4];
      p->pCores[cpuIdx].inaOffs     = (uint32_t) tmpRead[5];
      
      if(p->pCores[cpuIdx].actOffs < p->pCores[cpuIdx].inaOffs) { buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_A] = 1;
                                                                  buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_B] = 0;} 
      else                                                      { buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_A] = 0;
                                                                  buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_B] = 1;}
                                                                  
      buff[offset + cpuIdx*coreStateSize  + CPU_THR_RDY_A] = 1;
      buff[offset + cpuIdx*coreStateSize  + CPU_THR_RDY_B] = 1;
    
      thrIdx = 0;
      buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_STATUS]  = buff[offset + cpuIdx*coreStateSize  + CPU_STATUS];  
      buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_MSGS]    = buff[offset + cpuIdx*coreStateSize  + CPU_MSGS];
      
      if(p->thrQty > 1) {
        for(thrIdx=1;thrIdx < p->thrQty;thrIdx++) {
          buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_STATUS]  = 0;
          buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_MSGS]    = 0;
        }
      }
    } else {
      memset((uint8_t*)&buff[offset + cpuIdx*coreStateSize  + 0], 0, coreStateSize*4);
    }
  }


  return returnedLen;
}


//this is horrible code, but harmless. Does the job for now.
//TODO: replace this with something more sensible
void ftmShowStatus(uint32_t srcCpus, uint32_t* status, uint8_t verbose) {
  uint32_t cpuIdx, i;
  uint32_t ftmStatus, ftmMsgs, mySharedMem, sharedMem;
  uint32_t cfg;
  uint64_t tmp;
  long long unsigned int ftmTPrep;
  uint32_t* buffEbm   = status;
  uint32_t* buffPrioq = (uint32_t*)&buffEbm[((EBM_SEMA_RW)>>2)];
  uint32_t* buffWr    = (uint32_t*)&buffPrioq[((PRIO_CNT_OUT_ALL_GET_1 + 4)>>2)];
  uint32_t* buffCpu   = (uint32_t*)&buffWr[WR_STATE_SIZE];
  uint32_t coreStateSize = (CPU_STATE_SIZE + p->thrQty * THR_STATE_SIZE);
  char strBuff[65536];
  char* pSB = (char*)&strBuff;
  char sLinkState[20];
  char* pL = (char*)&sLinkState;
  char sSyncState[20];
  char* pS = (char*)&sSyncState;


  if(verbose) {
    //Generate EBM Status
     
    SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
    SNTPRINTF(pSB ,"\u2502 %sEBM%s                                                                           \u2502\n", KCYN, KNRM);
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 Status       \u2502 0x%08x",  buffEbm[EBM_STATUS_GET>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Src Mac      \u2502 0x%04x%08x", buffEbm[EBM_SRC_MAC_RW_1>>2],  buffEbm[EBM_SRC_MAC_RW_0>>2]); for(i=0;i<49;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Src IP       \u2502 0x%08x", buffEbm[EBM_SRC_IP_RW>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Src Port     \u2502 0x%04x", buffEbm[EBM_SRC_PORT_RW>>2]); for(i=0;i<57;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n"); 
    SNTPRINTF(pSB ,"\u2502 Dst Mac      \u2502 0x%04x%08x", buffEbm[EBM_DST_MAC_RW_1>>2],  buffEbm[EBM_DST_MAC_RW_0>>2]); for(i=0;i<49;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Dst IP       \u2502 0x%08x", buffEbm[EBM_DST_IP_RW>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Dst Port     \u2502 0x%04x", buffEbm[EBM_DST_PORT_RW>>2]); for(i=0;i<57;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 MTU          \u2502 %10u", buffEbm[EBM_MTU_RW>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Adr Hi       \u2502 0x%08x", buffEbm[EBM_ADR_HI_RW>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 EB Opt       \u2502 0x%08x", buffEbm[EBM_EB_OPT_RW>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2514"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
    
    //Generate PrioQ Status
    SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
    SNTPRINTF(pSB ,"\u2502 %sFPQ%s                                                                           \u2502\n", KCYN, KNRM);
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    cfg = buffPrioq[PRIO_MODE_GET>>2];
    SNTPRINTF(pSB ,"\u2502 Flags        \u2502 ");
    if(cfg & 0x1) SNTPRINTF(pSB ,"    ENA   ");  else SNTPRINTF(pSB ,"     -    ");
    if(cfg & 0x2) SNTPRINTF(pSB ," AFL_MSGS ");  else SNTPRINTF(pSB ,"     -    ");    
    if(cfg & 0x4) SNTPRINTF(pSB ," AFL_TIME ");  else SNTPRINTF(pSB ,"     -    ");
    for(i=0;i<33;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 Dst Adr      \u2502         0x%08x", buffPrioq[PRIO_ECA_ADR_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 EBM Adr      \u2502         0x%08x", buffPrioq[PRIO_EBM_ADR_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    tmp = (((uint64_t)buffPrioq[PRIO_CNT_OUT_ALL_GET_1>>2]) <<32) + ((uint64_t)buffPrioq[PRIO_CNT_OUT_ALL_GET_0>>2]); 
   
    SNTPRINTF(pSB ,"\u2502 Msgs Out     \u2502 %18llu", (long long unsigned int)tmp); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 TGather      \u2502 %18u", buffPrioq[PRIO_TX_MAX_WAIT_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 msg max      \u2502 %18u", buffPrioq[PRIO_TX_MAX_MSGS_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2514"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
  }

  //Generate WR Status
  uint64_t wrnow = (((uint64_t)buffWr[WR_UTC_HI]) << 32 | ((uint64_t)buffWr[WR_UTC_LO]));
  //55d32f8f
  
  if(buffWr[WR_STATUS] & PPS_VALID) SNTPRINTF(pL ,"  %sOK%s  ", KGRN, KNRM);
  else                              SNTPRINTF(pL ,"  %s--%s  ", KRED, KNRM);
  if(buffWr[WR_STATUS] & TS_VALID)  {
    //check if it's at least 1980something
    if (wrnow & 0x40000000ull) SNTPRINTF(pS ,"  %sOK%s  ", KGRN, KNRM); 
    else                        SNTPRINTF(pS ,"  %s<<%s  ", KYEL, KNRM);
   }  
  else                              SNTPRINTF(pS ,"  %s--%s  ", KRED, KNRM);
  
  time_t wrtime = (time_t)wrnow;
  SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
  SNTPRINTF(pSB ,"\u2502 %sWR %s                                                                           \u2502\n", KCYN, KNRM);
  SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
  SNTPRINTF(pSB ,"\u2502 PPS: %s TS: %s \u2502 WR-UTC: %.24s                     \u2502\n", sLinkState, sSyncState, ctime((time_t*)&wrtime));
  SNTPRINTF(pSB ,"\u2514"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");

  if (show_time)
  {
    SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
    SNTPRINTF(pSB ,"\u2502 %sRAW TIME %s                                                                     \u2502\n", KCYN, KNRM);
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 ECA TIME:              \u2502 0x%.8x%.8x                                   \u2502\n", currentTimeHigh, currentTimeLow);
    SNTPRINTF(pSB ,"\u2514"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
  }
    
  //Generate CPUs Status
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((srcCpus >> cpuIdx) & 0x1) {
      ftmStatus = buffCpu[cpuIdx*coreStateSize + CPU_STATUS];  
      ftmMsgs   = buffCpu[cpuIdx*coreStateSize + CPU_MSGS];
      sharedMem = buffCpu[cpuIdx*coreStateSize + CPU_SHARED]; //convert lm32's view to pcie's view
      mySharedMem = p->clusterAdr + (sharedMem & 0x3fffffff); //convert lm32's view to pcie's view
      ftmTPrep = (long long unsigned int)(((uint64_t)buffCpu[cpuIdx*coreStateSize + CPU_TPREP_HI]) << 32 | ((uint64_t)buffCpu[cpuIdx*coreStateSize + CPU_TPREP_LO]));
      
      SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
      SNTPRINTF(pSB ,"\u2502 %sCore #%02u%s                                                                      \u2502\n", KCYN, cpuIdx, KNRM);
      SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
      SNTPRINTF(pSB ,"\u2502 Status: %02x ErrCnt: %3u \u2502   MsgCnt: %9u       TPrep: %13llu ns    \u2502\n", \
       (uint8_t)ftmStatus, (uint8_t)(ftmStatus >> 8), ftmMsgs, ftmTPrep);
      SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
      SNTPRINTF(pSB ,"\u2502 Shared Mem: 0x%08x \u2502", mySharedMem + cpuIdx*CPU_SHARED_SIZE);
      if(p->pCores[cpuIdx].actOffs < p->pCores[cpuIdx].inaOffs) SNTPRINTF(pSB ,"   Act Page: A 0x%08x  Inact Page: B 0x%08x", p->pCores[cpuIdx].actOffs, p->pCores[cpuIdx].inaOffs);
      else                      SNTPRINTF(pSB ,"   Act Page: B 0x%08x  Inact Page: A 0x%08x", p->pCores[cpuIdx].actOffs, p->pCores[cpuIdx].inaOffs);
      SNTPRINTF(pSB ,"   \u2502\n");
      SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
      SNTPRINTF(pSB ,"\u2502       ");

      if(ftmStatus & STAT_RUNNING)    SNTPRINTF(pSB ,"   %sRUNNING%s   ", KGRN, KNRM);  else SNTPRINTF(pSB ,"   %sSTOPPED%s   ", KRED, KNRM);
      if(ftmStatus & STAT_IDLE)       SNTPRINTF(pSB ,"     %sIDLE%s    ", KYEL, KNRM);  else SNTPRINTF(pSB ,"     %sBUSY%s    ", KGRN, KNRM);
      if(ftmStatus & STAT_STOP_REQ)   SNTPRINTF(pSB ,"   STOP_REQ  ");  else SNTPRINTF(pSB ,"      -      ");
      if(ftmStatus & STAT_ERROR)      SNTPRINTF(pSB ,"     %sERROR%s   ", KRED, KNRM);  else SNTPRINTF(pSB ,"     %sOK%s      ", KGRN, KNRM);
      if(ftmStatus & STAT_WAIT)       SNTPRINTF(pSB ,"  WAIT_COND  ");  else SNTPRINTF(pSB ,"      -      ");
      SNTPRINTF(pSB ,"       \u2502\n");
      SNTPRINTF(pSB ,"\u2514"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
    }
  }
  printf("%s", (const char*)strBuff);
}

uint64_t cpus2thrs(uint32_t cpus) {
  uint64_t i;
  uint64_t res=0;
  
  for(i=0;i<8;i++) {
    res |= (((cpus >> i) & 1ull) << (i*8));
  }  
  return res;
}

uint32_t thrs2cpus(uint64_t thrs) {
  uint32_t i;
  uint64_t res=0;
  
  for(i=0;i<64;i++) res |= (((thrs >> i) & 1) << (i/8));
  return res;
  
}

int ftmFetchStatus(uint32_t* buff, uint32_t len)      { return v02FtmFetchStatus(buff, len);}
int ftmCommand(uint64_t dstThr, uint32_t command)     { return v02FtmCommand(thrs2cpus(dstThr),command);}
int ftmPutString(uint64_t dstThr, const char* sXml)   { return v02FtmPutString(thrs2cpus(dstThr), sXml);}
int ftmPutFile(uint64_t dstThr, const char* filename) { return v02FtmPutFile(thrs2cpus(dstThr), filename);}
int ftmCheckString(const char* sXml)                  { return v02FtmCheckString(sXml);}
int ftmClear(uint64_t dstThr)                         { return v02FtmClear(thrs2cpus(dstThr));}
int ftmDump(uint64_t srcThr, uint32_t len, uint8_t actIna, char* stringBuf, uint32_t lenStringBuf) 
                                                      { return v02FtmDump(thrs2cpus(srcThr), len, actIna, stringBuf, lenStringBuf);}
int ftmSetBp(uint64_t dstThr, int32_t planIdx)        { return v02FtmSetBp(thrs2cpus(dstThr), planIdx);}
  
