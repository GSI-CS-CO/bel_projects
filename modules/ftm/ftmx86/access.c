#include "access.h"

const uint64_t vendID_CERN       = 0x000000000000ce42;
const uint64_t vendID_GSI        = 0x0000000000000651;


const uint32_t devID_Reset       = 0x3a362063;
const uint32_t devID_RAM         = 0x66cfeb52;
const uint32_t devID_CoreRAM     = 0x54111351;
const uint32_t devID_SharedRAM   = 0x81111444;

const uint32_t devID_ClusterInfo = 0x10040086;
const uint32_t devID_ClusterCB   = 0x10041000;
const uint32_t devID_Ebm         = 0x00000815;
const uint32_t devID_Prioq       = 0x10040200;

t_ftmAccess* openFtm(const char* netaddress, t_ftmAccess* p, uint8_t overrideFWcheck)
{
  eb_cycle_t cycle;
  eb_status_t status;
  int cpuIdx, idx;
  int attempts;
  int num_devices;
  struct sdb_device devices[MAX_DEVICES];
  char              devName_RAM_post[4];
  struct sdb_bridge CluCB;
  const char myVer[] = "1.0.0\n";
  const char myName[] = "ftm\n";
  
  eb_data_t tmpRead[4];
  
  attempts   = 3;
  idx        = -1;

  /* open EB socket and device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &mySocket))               != EB_OK) die(status, "failed to open Etherbone socket");
  if ((status = eb_device_open(mySocket, netaddress, EB_ADDR32 | EB_DATA32, attempts, &device)) != EB_OK) die(status, "failed to open Etherbone device");

  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_ClusterCB, (struct sdb_device*)&CluCB, &num_devices)) != EB_OK)
  die(status, "failed to when searching for device");
  p->clusterAdr = CluCB.sdb_component.addr_first;
  
  //find reset ctrl
  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_Reset, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no reset controller found\n", program);
    goto error;
  }
  p->resetAdr = (eb_address_t)devices[0].sdb_component.addr_first;

  //get clusterInfo
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, vendID_GSI, devID_ClusterInfo, &devices[0], &num_devices, &CluCB)) != EB_OK)
  die(status, "failed to when searching for device");
  if (num_devices == 0) {
    fprintf(stderr, "%s: No lm32 clusterId rom found\n", program);
    goto error;
  }

  //get number of CPUs
  status = eb_device_read(device, (eb_address_t)devices[0].sdb_component.addr_first, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0], 0, eb_block);
  if (status != EB_OK) die(status, "failed to create cycle");
  p->cpuQty = (uint8_t)tmpRead[0];
  p->pCores =  malloc(p->cpuQty * sizeof(t_core));

  // Get Shared RAM
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity_at(device, vendID_GSI, devID_SharedRAM, &devices[0], &num_devices, &CluCB)) != EB_OK)
  die(status, "failed to when searching for Shared RAM ");
  //Old or new Gateware ?
  //FIXME: the cumbersome legacy code has to go sometime
  if(num_devices < 1) {
    //Old
    if ((status = eb_sdb_find_by_identity_at(device, vendID_CERN, devID_RAM, &devices[0], &num_devices, &CluCB)) != EB_OK)
    die(status, "failed to when searching for Shared RAM ");
  }
  p->sharedAdr = (eb_address_t)devices[0].sdb_component.addr_first;

  // Get prioq
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_Prioq, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for Priority Queue ");
  if (num_devices == 0) {
    fprintf(stderr, "%s: No Priority Queue found\n", program);
    goto error;
  }
  p->prioQAdr = (eb_address_t)devices[0].sdb_component.addr_first;
  
  // Get EBM
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_Ebm, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for device");
  if (num_devices == 0) {
    fprintf(stderr, "%s: No Etherbone Master found\n", program);
    goto error;
  }
  p->ebmAdr = (eb_address_t)devices[0].sdb_component.addr_first;
  
  //Get RAMs 
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, vendID_GSI, devID_CoreRAM, &devices[0], &num_devices, &CluCB)) != EB_OK)
  die(status, "failed to when searching for device");
  
  //Old or new Gateware ?
  //FIXME: the cumbersome legacy code has to go sometime
  if(overrideFWcheck) printf("Gate-/firmware check disabled by override option\n");
  
  if(num_devices > 0) {
    //new
    ftm_shared_offs = FTM_SHARED_OFFSET_NEW;
    for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      p->pCores[cpuIdx].ramAdr = devices[cpuIdx].sdb_component.addr_first;
      //check for valid firmware
      if(overrideFWcheck) p->pCores[cpuIdx].hasValidFW = 1;
      else                p->pCores[cpuIdx].hasValidFW = isFwValid(&devices[cpuIdx], &myVer[0], &myName[0]);
    }
  } else {
    //Old
    ftm_shared_offs = FTM_SHARED_OFFSET_OLD;
    if(!overrideFWcheck) {
      printf("ERROR: FTM is using old gate-/firmware. Sure this is the FTM you want ? Use option '-o' if you want to override\n");
      goto error;
    } else printf("WARNING: FTM is using old gateware. Be sure you know what you're doing\n");
    
    for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      devName_RAM_post[0] = '0';
      devName_RAM_post[1] = '0' + (p->cpuQty & 0xf);
      devName_RAM_post[2] = '0' + (cpuIdx  & 0xf);
      devName_RAM_post[3] =  0;

      num_devices = MAX_DEVICES;
      if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
      die(status, "failed to when searching for device");
      
      for (idx = 0; idx < num_devices; ++idx) {
        if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0) {
          p->pCores[cpuIdx].ramAdr = devices[idx].sdb_component.addr_first;
          p->pCores[cpuIdx].hasValidFW = 1;
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
    
    } else printf("Core #%u: Can't read core offsets - no valid firmware present.\n", cpuIdx);
  }


  return p;

  error:
  closeFtm();
  return p; //dummy
}

void closeFtm(void)
{
  eb_status_t status;
  if ((status = eb_device_close(device))   != EB_OK) die(status, "failed to close Etherbone device");
  if ((status = eb_socket_close(mySocket)) != EB_OK) die(status, "failed to close Etherbone socket");
  exit(-1);
}

int die(eb_status_t status, const char* what)
{  
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  exit(1);
}


uint8_t isFwValid(struct  sdb_device* ram, const char* sVerExp, const char* sName)
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
  if(!validity) printf("No firmware found!\n");

  //check project
  pos = strstr(cBuff, "Project     : ");
  if(pos != NULL) {
    pos += 14;
    if(strncmp(pos, sName, strlen(sName))) {validity = 0;} 
  } else { printf("This is no ftm firmware, name does not match!\n");}
  
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
    printf("WARNING:  Expected firmware version %u.%u.%u, but found only %u.%u.%u!\n", verExpMaj, verExpMin, verExpRev, verFndMaj, verFndMin, verFndRev);  
  }
  if(verExp < verFnd ) {
    printf("WARNING:  Expected firmware version %u.%u.%u is lower than found version %u.%u.%u\n", verExpMaj, verExpMin, verExpRev, verFndMaj, verFndMin, verFndRev);
  }
  
  //no fwid found. try legacy
        
  return validity;
    
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

void ebRamClear(uint32_t address, uint32_t len)
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
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, 0); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close write cycle");
      start = start + partLen;
   }
}
