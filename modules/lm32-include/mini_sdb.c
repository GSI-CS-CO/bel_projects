#include <stdio.h>
#include "mini_sdb.h"
#include "dbg.h"
#include "hw/memlayout.h"
#include "sdb_add.h"

sdb_location *find_sdb_deep(sdb_record_t *parent_sdb, sdb_location *found_sdb, uint32_t base, uint32_t msi_base,  uint32_t msi_last, uint32_t *idx, uint32_t qty, uint32_t venId, uint32_t devId)
{
  sdb_record_t *record;
  uint32_t records;
  uint32_t i;
  uint32_t msi_cnt = 0;
  uint32_t msi_adr = 0;
    
  

  record  = parent_sdb;
  records = record->interconnect.sdb_records;
  //discover MSI address before moving on to possible next Crossbar
  for (i = 0; i < records; ++i, ++record) {
    
    if(record->empty.record_type == SDB_MSI) {

      if (record->msi.msi_flags & OWN_MSI) {
        //mprintf("adr: 0x%08x, i : %u type %u own: 0x%08x msi_addr %08x \n", base, i, record->empty.record_type, record->msi.msi_flags, record->msi.sdb_component.addr_first.low);  
        if((msi_base == NO_MSI) || (record->msi.sdb_component.product.vendor_id.low == 0 && record->msi.sdb_component.product.device_id == 0)) msi_base = NO_MSI;
        else msi_adr = record->msi.sdb_component.addr_first.low;
        msi_cnt++;
      }  
    }
  }
  if( msi_cnt > 1) {
  //This is an error, the CB layout is messed up
    DBPRINT1("Found more than 1 MSI at 0x%08x par 0x%08x\n", base, (uint32_t)(unsigned char*)parent_sdb);
    *idx = 0;      
    return found_sdb;
  }

  record  = parent_sdb;
  records = record->interconnect.sdb_records;
  for (i = 0; i < records; ++i, ++record) {
     if (record->empty.record_type == SDB_BRIDGE) {

        if (record->bridge.sdb_component.product.vendor_id.low == venId &&
        record->bridge.sdb_component.product.device_id == devId) {
           DBPRINT2("Target BRG at base 0x%08x 0x%08x  entry %u\n", base, base+record->bridge.sdb_component.addr_first.low, *idx);
           found_sdb[(*idx)].sdb = record;
           found_sdb[(*idx)].adr = base;
           found_sdb[(*idx)].msi_first = msi_base + msi_adr;
           found_sdb[(*idx)].msi_last  = msi_base + msi_adr + msi_last;   
           (*idx)++;
        }  

        find_sdb_deep((sdb_record_t *)(base+record->bridge.sdb_child.low), found_sdb, base+record->bridge.sdb_component.addr_first.low, msi_base+msi_adr, msi_last, idx, qty, venId, devId );
     }
     
     

     if (record->empty.record_type == SDB_DEVICE) {
        if (record->device.sdb_component.product.vendor_id.low == venId &&
        record->device.sdb_component.product.device_id == devId) {
           DBPRINT2("Target DEV at 0x%08x\n", base + record->device.sdb_component.addr_first.low);
           found_sdb[(*idx)].sdb = record;
           found_sdb[(*idx)].adr = base;
           found_sdb[(*idx)].msi_first = msi_base + msi_adr;
           found_sdb[(*idx)].msi_last  = msi_base + msi_adr + msi_last; 
           (*idx)++;
        }
     }

     //This gets us addr_last. We need it for the upper range when programming an MSI master destination
     if (record->empty.record_type == SDB_MSI) {
        if (record->msi.sdb_component.product.vendor_id.low == venId &&
        record->msi.sdb_component.product.device_id == devId) {
           DBPRINT2("Target MSI at 0x%08x\n", base + record->msi.sdb_component.addr_first.low);
           found_sdb[(*idx)].sdb = record;
           found_sdb[(*idx)].adr = base;
           found_sdb[(*idx)].msi_first = msi_base + msi_adr;
           found_sdb[(*idx)].msi_last  = msi_base + msi_adr + msi_last;  
           (*idx)++;
        }
     } 
     if(*idx >= qty) {
      return found_sdb;
    }
  }
  
  return found_sdb;
}

uint32_t getMsiUpperRange() {
  sdb_record_t *record = (sdb_record_t *)((uint32_t)(sdb_add()));
  uint32_t records    = record->interconnect.sdb_records;
  uint32_t i;
  uint32_t msi_adr    = 0;
  
  //get upper range of MSI target  
  for (i = 0; i < records; ++i, ++record) {  
    if(record->empty.record_type == SDB_MSI) {
      if (record->msi.msi_flags == OWN_MSI) {
        msi_adr = record->msi.sdb_component.addr_last.low;
        break;  
      }  
    }
  } 

  return msi_adr;
}






// convenience wrappers
sdb_location* find_device_multi(sdb_location *found_sdb, uint32_t *idx, uint32_t qty, uint32_t venId, uint32_t devId)
{
  uint32_t root = sdb_add();  
  sdb_record_t *pRoot = (sdb_record_t *)((uint32_t)(root));


  return find_sdb_deep(pRoot, found_sdb, 0, 0, getMsiUpperRange(), idx, qty, venId, devId);
  
}

uint32_t* find_device_adr(uint32_t venId, uint32_t devId)
{
   sdb_location found_sdb;
   uint32_t idx = 0;
   uint32_t* adr = (uint32_t*)ERROR_NOT_FOUND;
   
   find_device_multi(&found_sdb, &idx, 1, venId, devId);
   if(idx > 0) adr = (uint32_t*)getSdbAdr(&found_sdb);
   
   return adr;
}


sdb_location* find_device_multi_in_subtree(sdb_location *loc, sdb_location *found_sdb, uint32_t *idx, uint32_t qty, uint32_t venId, uint32_t devId)
{
   //return find_sdb_deep(getChild(loc), found_sdb, getSdbAdr(loc), getMsiAdr(loc), getMsiUpperRange(), idx, qty, venId, devId);
   return find_sdb_deep(getChild(loc), found_sdb, getSdbAdr(loc), getMsiAdr(loc), getMsiUpperRange(), idx, qty, venId, devId);
}

uint32_t* find_device_adr_in_subtree(sdb_location *loc, uint32_t venId, uint32_t devId)
{
   sdb_location found_sdb;
   uint32_t idx = 0;
   uint32_t* adr = (uint32_t*)ERROR_NOT_FOUND;
   find_sdb_deep(getChild(loc), &found_sdb, getSdbAdr(loc), getMsiAdr(loc), getMsiUpperRange(), &idx, 1, venId, devId);
   if(idx > 0) adr = (uint32_t*)getSdbAdr(&found_sdb);

   return adr;
}


uint32_t getSdbAdr(sdb_location *loc)
{
   if       (loc->sdb->empty.record_type == SDB_DEVICE ) return loc->adr + loc->sdb->device.sdb_component.addr_first.low;
   else if  (loc->sdb->empty.record_type == SDB_BRIDGE ) return loc->adr + loc->sdb->bridge.sdb_component.addr_first.low;
   else return ERROR_NOT_FOUND;
}

uint32_t getMsiAdr(sdb_location *loc)
{
   return loc->msi_first;
   
}

uint32_t getMsiAdrLast(sdb_location *loc)
{
   return loc->msi_last;
   
}

uint32_t getSdbAdrLast(sdb_location *loc)
{
   if      (loc->sdb->empty.record_type == SDB_DEVICE ) return loc->adr + loc->sdb->device.sdb_component.addr_last.low;
   else if (loc->sdb->empty.record_type == SDB_BRIDGE ) return loc->adr + loc->sdb->bridge.sdb_component.addr_last.low;
   else return ERROR_NOT_FOUND;
}




sdb_record_t* getChild(sdb_location *loc)
{
   return (sdb_record_t*)(loc->adr + loc->sdb->bridge.sdb_child.low);
}

//DEPRECATED, USE find_device_adr INSTEAD!
uint8_t* find_device(uint32_t devid)
{
        return (unsigned char *)find_device_adr(GSI, devid);
}



void discoverPeriphery(void)
{
  sdb_location found_sdb[20];
  sdb_location found_sdb_w1[2];
  uint32_t idx = 0;
  uint32_t idx_w1 = 0;
  pCpuMsiBox      = NULL;
  pMyMsi          = NULL; 

  pUart           = find_device_adr(CERN, WR_UART);
  //pUart          = (uint32_t*)0x84060500;
  BASE_UART       = (unsigned char *)pUart; //make WR happy ...
  
 
  pCpuId          = find_device_adr(GSI, CPU_INFO_ROM);
  pCpuAtomic      = find_device_adr(GSI, CPU_ATOM_ACC);
  pCpuSysTime     = find_device_adr(GSI, CPU_SYSTEM_TIME);
  pCpuIrqSlave    = find_device_adr(GSI, CPU_MSI_CTRL_IF);

  idx = 0;

  find_device_multi(&found_sdb[0], &idx, 1, GSI, MSI_MSG_BOX);   
  if(idx) {
    pCpuMsiBox    = (uint32_t*)getSdbAdr(&found_sdb[0]); 
    pMyMsi        = (uint32_t*)getMsiAdr(&found_sdb[0]); 
  } 
  pCluCB          = find_device_adr(GSI, LM32_CB_CLUSTER);
  pCluInfo        = find_device_adr(GSI, CLU_INFO_ROM);
  pFpqCtrl        = find_device_adr(GSI, FTM_PRIOQ_CTRL); 
  pFpqData        = find_device_adr(GSI, FTM_PRIOQ_DATA); 
  
    
  pOledDisplay    = find_device_adr(GSI, OLED_DISPLAY);  
  idx = 0;
  find_device_multi(&found_sdb[0], &idx, 20, GSI, ETHERBONE_MASTER);
  pEbm            = (uint32_t*)getSdbAdr(&found_sdb[0]);
  pEbmLast        = (uint32_t*)getSdbAdrLast(&found_sdb[0]);
  pEbCfg          = find_device_adr(GSI, ETHERBONE_CFG);
  pEca            = find_device_adr(GSI, ECA_EVENT);
  pTlu            = find_device_adr(GSI, TLU);

  
  pCfiPFlash      = find_device_adr(GSI, WR_CFIPFlash);
  
  pDDR3_if1       = find_device_adr(GSI, WB_DDR3_if1);
  pDDR3_if2       = find_device_adr(GSI, WB_DDR3_if2);
  
  // Get the second onewire/w1 record (0=white rabbit w1 unit, 1=user w1 unit)
  find_device_multi(&found_sdb_w1[0], &idx_w1, 2, CERN, WR_1Wire);
  pOneWire        = (uint32_t*)getSdbAdr(&found_sdb_w1[1]);

  BASE_SYSCON     = (unsigned char *)find_device_adr(CERN, WR_SYS_CON);
  pPps            = find_device_adr(CERN, WR_PPS_GEN);

}
