#include "mini_sdb.h"
#include "aux.h"

sdb_location *find_sdb_deep(sdb_record_t *parent_sdb, sdb_location *found_sdb, unsigned int base, unsigned int *idx, unsigned int qty, unsigned int venId, unsigned int devId)
{
        sdb_record_t *record = parent_sdb;
        int records = record->interconnect.sdb_records;
        int i;
        DBPRINT1("base 0x%08x\n", base);
           for (i = 0; i < records; ++i, ++record) {
                   if (record->empty.record_type == SDB_BRIDGE) {
                     
                     if (record->bridge.sdb_component.product.vendor_id.low == venId &&
                         record->bridge.sdb_component.product.device_id == devId) {
                           DBPRINT1("Target BRG at base 0x%08x 0x%08x  entry %u\n", base, base+record->bridge.sdb_component.addr_first.low, *idx);
                           found_sdb[(*idx)].sdb = record;
                           found_sdb[(*idx)].adr = base;
                           (*idx)++;
                           
        
                     }  
                     DBPRINT2("call base %08x sdb %08x Adr: 0x%08x \n", base, (sdb_record_t *)(base +record->bridge.sdb_child.low), base+record->bridge.sdb_component.addr_first.low);
                     
                     find_sdb_deep((sdb_record_t *)(base+record->bridge.sdb_child.low), found_sdb, base+record->bridge.sdb_component.addr_first.low, idx, qty, venId, devId);
                     
                   }
                   
                   if (record->empty.record_type == SDB_DEVICE) {
                      DBPRINT3("idx %u max %u Adr: 0x%08x DEV VEN 0x%08x ID 0x%08x\n", i, records, base+record->device.sdb_component.addr_first.low, record->device.sdb_component.product.vendor_id.low, record->device.sdb_component.product.device_id);
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

sdb_location find_sdb(unsigned int venId, unsigned int devId)
{
   sdb_location ret;
   unsigned int idx = 0;
   find_sdb_deep((sdb_record_t *)((unsigned int)(SBD_BASE)), &ret, 0, &idx, 1, venId, devId);
   return ret;
}

unsigned int getSdbAdr(sdb_location loc)
{
   if (loc.sdb->empty.record_type == SDB_DEVICE ) return loc.adr + loc.sdb->device.sdb_component.addr_first.low;
   else return loc.adr + loc.sdb->bridge.sdb_component.addr_first.low;
}

unsigned char *find_device_deep(unsigned int base, unsigned int sdb,
                                       unsigned int devid)
{
        sdb_record_t *record = (sdb_record_t *) sdb;
        int records = record->interconnect.sdb_records;
        int i;

        for (i = 0; i < records; ++i, ++record) {
                if (record->empty.record_type == SDB_BRIDGE) {
                                                
                        unsigned char *out =
                            find_device_deep(base +  record->bridge.sdb_component.
					                                      addr_first.low,
					                                      base + record->bridge.sdb_child.low,
					                                      devid);
                        if (out)
                                return out;
                }
                if (record->empty.record_type == SDB_DEVICE &&
                    record->device.sdb_component.product.device_id == devid) {
                        break;
                }
        }

        if (i == records)
                return 0;
        return (unsigned char *)(base +
                                 record->device.sdb_component.addr_first.low);
}

unsigned char *find_device(unsigned int devid)
{
        return find_device_deep(0, (unsigned int)(SBD_BASE), devid);
}

void discoverPeriphery()
{
   pCpuId         = (unsigned int*)find_device(CPU_INFO_ROM);
   pCpuAtomic     = (unsigned int*)find_device(CPU_ATOM_ACC);
   pCluInfo       = (unsigned int*)find_device(CPU_CLU_INFO_ROM);
   pCpuSysTime    = (unsigned int*)find_device(CPU_SYSTEM_TIME);
   
   pCpuIrqSlave   = (unsigned int*)find_device(IRQ_MSI_CTRL_IF);   
   pCpuTimer      = (unsigned int*)find_device(IRQ_TIMER_CTRL_IF);
   
   pFpqCtrl       = (unsigned int*)find_device(FTM_PRIOQ_CTRL); 
   pFpqData       = (unsigned int*)find_device(FTM_PRIOQ_DATA); 
   
   pOledDisplay   = (unsigned int*)find_device(OLED_DISPLAY);  
   pEbm           = (unsigned int*)find_device(ETHERBONE_MASTER);
   pEca           = (unsigned int*)find_device(ECA_EVENT);
   pUart          = (unsigned int*)find_device(WR_UART);
   BASE_UART      = pUart; //make WR happy ...
}

