#ifndef GSI_SDB_NEORV32_H
#define GSI_SDB_NEORV32_H

#include <stdint.h>

#define GSI_SDB_ANY UINT32_MAX
#define GSI_SDB_DEPTH 16

#define GSI_SDB_GSI_VENDOR_ID  0x00000651
#define GSI_SDB_CERN_VENDOR_ID 0x0000CE42

/* Number of addresses stored, not an input capacity. */
typedef uint32_t devices;

/* Decoded record, including the path used by eb-ls (for example 13.2.1).
 * Names are NUL-terminated here, unlike the on-bus SDB name field. */
typedef struct
{
  uint64_t vendor_id;
  uint32_t device_id;
  uint32_t address;
  uint32_t msi_flags;
  uint16_t path[GSI_SDB_DEPTH];
  uint8_t depth;
  uint8_t type;
  char name[20];
} gsi_sdb_entry;

typedef int (*gsi_sdb_visitor)(const gsi_sdb_entry *, void *);

/* Return 0: found, 1: no match, 2: invalid SDB/arguments or full result array.
 * On error *devices_found is zero; discard array contents.
 * Wildcards include devices, bridges and MSI records, as shown by eb-ls.
 * Vendor IDs are 64-bit in SDB; this API matches a zero-extended 32-bit ID. */
uint32_t gsi_sdb_find_device(uint32_t array[], uint32_t venId, uint32_t devId,
                             devices *devices_found, uint32_t max_devices);

/* Visit all non-interconnect records in depth-first order.
 * Return 0 on success or 2 on error; a nonzero visitor return aborts the scan. */
uint32_t gsi_sdb_walk(gsi_sdb_visitor visitor, void *context);


/* Platform helper: read the SDB root address from the GPIO input vector. */
uint32_t gsi_sdb_get_root(void);

#endif
