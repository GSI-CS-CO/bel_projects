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
        return find_device_deep(0, (unsigned int)pSDB_base, devid);
}
