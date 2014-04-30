#include "mini_sdb.h"



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

