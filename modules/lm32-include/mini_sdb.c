#include <stdio.h>
#include "mini_sdb.h"
#include "dbg.h"

sdb_location *find_sdb_deep(sdb_record_t *parent_sdb, sdb_location *found_sdb, unsigned int base, unsigned int *idx, unsigned int qty, unsigned int venId, unsigned int devId)
{
     sdb_record_t *record = parent_sdb;
     int records = record->interconnect.sdb_records;
     int i;
     DBPRINT1("base 0x%08x par 0x%08x\n", base, (unsigned int)(unsigned char*)parent_sdb);
     for (i = 0; i < records; ++i, ++record) {
         if (record->empty.record_type == SDB_BRIDGE) {

            if (record->bridge.sdb_component.product.vendor_id.low == venId &&
            record->bridge.sdb_component.product.device_id == devId) {
               DBPRINT1("Target BRG at base 0x%08x 0x%08x  entry %u\n", base, base+record->bridge.sdb_component.addr_first.low, *idx);
               found_sdb[(*idx)].sdb = record;
               found_sdb[(*idx)].adr = base;
               (*idx)++;
            }  
            find_sdb_deep((sdb_record_t *)(base+record->bridge.sdb_child.low), found_sdb, base+record->bridge.sdb_component.addr_first.low, idx, qty, venId, devId);
         }

         if (record->empty.record_type == SDB_DEVICE) {
            if (record->device.sdb_component.product.vendor_id.low == venId &&
            record->device.sdb_component.product.device_id == devId) {
               DBPRINT1("Target DEV at 0x%08x\n", base + record->device.sdb_component.addr_first.low);
               found_sdb[(*idx)].sdb = record;
               found_sdb[(*idx)].adr = base;
               (*idx)++;
            }
         }
         if(*idx >= qty) return found_sdb;
      }
     
     return found_sdb;
}

// convenience wrappers
sdb_location* find_device_multi(sdb_location *found_sdb, unsigned int *idx, unsigned int qty, unsigned int venId, unsigned int devId)
{
   return find_sdb_deep((sdb_record_t *)((unsigned int)(SBD_BASE)), found_sdb, 0, idx, qty, venId, devId);
}

unsigned int* find_device_adr(unsigned int venId, unsigned int devId)
{
   sdb_location found_sdb;
   unsigned int idx = 0;
   unsigned int* adr;
   
   find_device_multi(&found_sdb, &idx, 1, venId, devId);
   if(idx > 0) adr = (unsigned int*)getSdbAdr(&found_sdb);
   else        adr = NULL;
   
   return adr;
}


sdb_location* find_device_multi_in_subtree(sdb_location *loc, sdb_location *found_sdb, unsigned int *idx, unsigned int qty, unsigned int venId, unsigned int devId)
{
   return find_sdb_deep(getChild(loc), found_sdb, getSdbAdr(loc), idx, qty, venId, devId);
}



unsigned int* find_device_adr_in_subtree(sdb_location *loc, unsigned int venId, unsigned int devId)
{
   sdb_location found_sdb;
   unsigned int idx = 0;
   unsigned int* adr;
   find_sdb_deep(getChild(loc), &found_sdb, getSdbAdr(loc), &idx, 1, venId, devId);
   if(idx > 0) adr = (unsigned int*)getSdbAdr(&found_sdb);
   else        adr = NULL;
   return adr;
}


unsigned int getSdbAdr(sdb_location *loc)
{
   if (loc->sdb->empty.record_type == SDB_DEVICE ) 
   {
      return loc->adr + loc->sdb->device.sdb_component.addr_first.low;
   }
   else return loc->adr + loc->sdb->bridge.sdb_component.addr_first.low;
}

sdb_record_t* getChild(sdb_location *loc)
{
   return (sdb_record_t*)(loc->adr + loc->sdb->bridge.sdb_child.low);
   
}

//DEPRECATED, USE find_device_adr INSTEAD!
unsigned char *find_device(unsigned int devid)
{
        return (unsigned char *)find_device_adr(GSI, devid);
}

void discoverPeriphery(void)
{
  sdb_location found_sdb[20];
  sdb_location found_sdb_w1[2];
  unsigned int idx = 0;
  unsigned int idx_w1 = 0;
   
   pCpuId         = find_device_adr(GSI, CPU_INFO_ROM);
   pCpuAtomic     = find_device_adr(GSI, CPU_ATOM_ACC);
   pCpuSysTime    = find_device_adr(GSI, CPU_SYSTEM_TIME);
   pCpuIrqSlave   = find_device_adr(GSI, IRQ_MSI_CTRL_IF);   
   pCpuTimer      = find_device_adr(GSI, IRQ_TIMER_CTRL_IF);
   
   //FIXME this should not count found std CBs, but use a CB with a special devId
   
   find_device_multi(&found_sdb[0], &idx, 20, GSI, CB_CLUSTER);
   //find_device_multi(&found_sdb[0], &idx, 20, GSI, CB_GENERIC);
   pCluCB         = (unsigned int*)getSdbAdr(&found_sdb[0]);
   pSharedRam     = find_device_adr_in_subtree(&found_sdb[0], CERN, DPRAM_GENERIC);
   pCluInfo       = find_device_adr_in_subtree(&found_sdb[0], GSI, CPU_CLU_INFO_ROM);
   pFpqCtrl       = find_device_adr_in_subtree(&found_sdb[0], GSI, FTM_PRIOQ_CTRL); 
   pFpqData       = find_device_adr_in_subtree(&found_sdb[0], GSI, FTM_PRIOQ_DATA); 
   
  pOledDisplay   = find_device_adr(GSI, OLED_DISPLAY);  
  pEbm           = find_device_adr(GSI, ETHERBONE_MASTER);
  pEca           = find_device_adr(GSI, ECA_EVENT);
  pTlu           = find_device_adr(GSI, TLU);
  pUart          = find_device_adr(CERN, WR_UART);
  BASE_UART      = pUart; //make WR happy ...
  pCfiPFlash     = find_device_adr(GSI, WR_CFIPFlash);
  
  /* Get the second onewire/w1 record (0=white rabbit w1 unit, 1=user w1 unit) */
  find_device_multi(&found_sdb_w1[0], &idx_w1, 2, CERN, WR_1Wire);
  pOneWire         = (unsigned int*)getSdbAdr(&found_sdb_w1[1]);

}

