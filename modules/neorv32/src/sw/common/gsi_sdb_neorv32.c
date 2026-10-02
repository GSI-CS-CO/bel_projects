#include "gsi_sdb_neorv32.h"

#include <stddef.h>
#include <sdb.h>

#include <neorv32.h>

/* The supplied SDB 1.1 header predates the MSI extension (record type 3). */
#define SDB_MSI 3
#define SDB_RECORD_BYTES 64
#define SDB_RECORD_BUDGET 4096

/* Every SDB record occupies 64 bytes. The union permits typed field access
 * while retaining aligned 32-bit reads for the Wishbone bus. */
union sdb_record
{
  uint32_t words[16];
  struct sdb_interconnect bus;
  struct sdb_device device;
  struct sdb_bridge bridge;
};

_Static_assert(sizeof(union sdb_record) == SDB_RECORD_BYTES, "SDB record size");
_Static_assert(offsetof(struct sdb_device, sdb_component.product.record_type) == 63,
               "SDB record type offset");

/* Scan state is local to each call: concurrent callers do not share buffers.
 * ancestors detects bridge cycles; path records the indices printed by eb-ls.
 * remaining bounds the amount of work even for malformed or very large trees. */
struct walk
{
  gsi_sdb_visitor visitor;
  void *context;
  uint32_t ancestors[GSI_SDB_DEPTH];
  uint32_t remaining;
  uint16_t path[GSI_SDB_DEPTH];
};

struct search
{
  uint32_t *array;
  uint32_t vendor;
  uint32_t device;
  uint32_t capacity;
  uint32_t count;
};

/* Read a record from the memory-mapped Wishbone bus. Physical access faults
 * require platform trap handling; this function provides no bus timeout. */
static int gsi_sdb_read_record(uint32_t address, uint32_t words[16])
{
  const volatile union sdb_record *record =
    (const volatile union sdb_record *)(uintptr_t)address;

  for (unsigned i = 0; i < 16; ++i)
  {
    words[i] = record->words[i];
  }

  return 0;
}


/* SDB fields use network byte order, independently of CPU byte order. */
static uint16_t big16(uint16_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap16(value);
#else
  return value;
#endif
}

static uint32_t big32(uint32_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap32(value);
#else
  return value;
#endif
}

static uint64_t big64(uint64_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}

/* A Wishbone adapter may reverse bytes within each 32-bit word. Normalize
 * that bus-specific ordering first; big16/32/64 then decode the SDB fields. */
static int read_record(uint32_t address, union sdb_record *record, int swap)
{
  if (gsi_sdb_read_record(address, record->words))
  {
    return 2;
  }

  if (swap)
  {
    for (unsigned i = 0; i < 16; ++i)
    {
      record->words[i] = __builtin_bswap32(record->words[i]);
    }
  }

  return 0;
}

/* Walk a table in depth-first eb-ls order. table is an absolute SDB address;
 * base is the absolute start of this bus's window in the parent address space.
 * Child table pointers and device addresses are relative to that bus base. */
static int walk_bus(struct walk *walk, uint32_t table, uint32_t base,
                    unsigned depth)
{
  union sdb_record record;

  if (depth >= GSI_SDB_DEPTH || (table & (SDB_RECORD_BYTES - 1)))
  {
    return 2;
  }

  /* Only ancestors matter: visiting a shared subtree is not itself a cycle. */
  for (unsigned i = 0; i < depth; ++i)
  {
    if (walk->ancestors[i] == table)
    {
      return 2;
    }
  }
  walk->ancestors[depth] = table;

  if (read_record(table, &record, 0))
  {
    return 2;
  }

  /* Detect the adapter's word ordering from the first record's magic value. */
  int swap = 0;

  if (big32(record.bus.sdb_magic) != SDB_MAGIC)
  {
    if (big32(__builtin_bswap32(record.bus.sdb_magic)) != SDB_MAGIC)
    {
      return 2;
    }

    swap = 1;
    for (unsigned i = 0; i < 16; ++i)
    {
      record.words[i] = __builtin_bswap32(record.words[i]);
    }
  }

  /* sdb_records includes the interconnect header. Check the table extent
   * in 64-bit arithmetic before calculating any 32-bit record address. */
  unsigned count = big16(record.bus.sdb_records);

  if (record.bus.sdb_component.product.record_type != sdb_type_interconnect ||
      record.bus.sdb_version != 1 ||
      record.bus.sdb_bus_type != sdb_wishbone ||
      count == 0 || count > walk->remaining ||
      (uint64_t)table + count * SDB_RECORD_BYTES > UINT64_C(0x100000000))
  {
    return 2;
  }
  walk->remaining -= count;

  /* eb-ls does not print the interconnect header, so start at record 1. */
  for (unsigned i = 1; i < count; ++i)
  {
    if (read_record(table + i * SDB_RECORD_BYTES, &record, swap))
    {
      return 2;
    }

    const struct sdb_component *component = &record.device.sdb_component;
    gsi_sdb_entry entry = {0};

    entry.type = component->product.record_type;
    entry.depth = depth + 1;
    walk->path[depth] = i;

    for (unsigned j = 0; j <= depth; ++j)
    {
      entry.path[j] = walk->path[j];
    }

    if (entry.type >= sdb_type_device && entry.type <= SDB_MSI)
    {
      uint64_t first = big64(component->addr_first);
      uint64_t last = big64(component->addr_last);

      /* MSI targets use a separate address space, so do not add bus base.
       * The active MSI bit also depends on which master reads the table. */
      uint64_t offset = entry.type == SDB_MSI ? 0 : base;

      if (first > last || first + offset > UINT32_MAX ||
          last > UINT32_MAX || last + offset > UINT32_MAX)
      {
        return 2;
      }

      entry.address = first + offset;
      entry.vendor_id = big64(component->product.vendor_id);
      entry.device_id = big32(component->product.device_id);
      entry.msi_flags = entry.type == SDB_MSI ? big32(record.words[0]) : 0;

      /* SDB names are 19-byte, blank-padded fields without a terminator.
       * entry was zero-initialized, leaving name[19] as the terminating NUL. */
      for (unsigned j = 0; j < 19; ++j)
      {
        entry.name[j] = component->product.name[j];
      }
    }
    else if (entry.type < 0x80)
    {
      /* Unknown hardware record types cannot safely be interpreted.
       * Informational records (bit 7 set), including empty slots, are allowed. */
      return 2;
    }

    /* The visitor also receives informational/empty records for a full list.
     * It may abort the scan, for example when a result array is full. */
    if (walk->visitor(&entry, walk->context))
    {
      return 2;
    }

    if (entry.type == sdb_type_bridge)
    {
      uint64_t child = big64(record.bridge.sdb_child);

      /* A bridge must contain its child SDB header. Its child pointer is
       * relative to the current bus, not relative to the bridge window. */
      if (child > UINT32_MAX || child + base > UINT32_MAX ||
          child < big64(component->addr_first) ||
          child + SDB_RECORD_BYTES - 1 > big64(component->addr_last))
      {
        return 2;
      }

      if (walk_bus(walk, child + base, entry.address, depth + 1))
      {
        return 2;
      }
    }
  }

  return 0;
}

/* Resolve the root through the platform GPIO helper, then scan from bus base 0. */
uint32_t gsi_sdb_walk(gsi_sdb_visitor visitor, void *context)
{
  if (!visitor)
  {
    return 2;
  }

  struct walk walk = {
    .visitor = visitor,
    .context = context,
    .remaining = SDB_RECORD_BUDGET
  };

  return walk_bus(&walk, gsi_sdb_get_root(), 0, 0);
}

/* Search visitor: only devices, bridges and MSI records carry addresses.
 * Each ID may independently be a wildcard; otherwise require an exact match. */
static int collect(const gsi_sdb_entry *entry, void *context)
{
  struct search *search = context;

  if (entry->type < sdb_type_device || entry->type > SDB_MSI)
  {
    return 0;
  }

  if ((search->vendor != GSI_SDB_ANY && entry->vendor_id != search->vendor) ||
      (search->device != GSI_SDB_ANY && entry->device_id != search->device))
  {
    return 0;
  }

  /* Check before storing: the caller's array must never overflow. */
  if (search->count == search->capacity)
  {
    return 2;
  }

  search->array[search->count++] = entry->address;
  return 0;
}

uint32_t gsi_sdb_find_device(uint32_t array[], uint32_t venId, uint32_t devId,
                           devices *devices_found, uint32_t max_devices)
{
  if (!devices_found)
  {
    return 2;
  }

  /* Publish a count only after a complete successful scan. If an error occurs,
   * callers must discard any addresses already written into the array. */
  *devices_found = 0;

  if (!array && max_devices)
  {
    return 2;
  }

  struct search search = {
    .array = array,
    .vendor = venId,
    .device = devId,
    .capacity = max_devices,
    .count = 0
  };

  if (gsi_sdb_walk(collect, &search))
  {
    return 2;
  }

  *devices_found = search.count;
  return search.count ? 0 : 1;
}

/* Get SDB root address */
uint32_t gsi_sdb_get_root(void)
{
  return neorv32_gpio_port_get();
}
