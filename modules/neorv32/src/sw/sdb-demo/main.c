#include <neorv32.h>
#include <string.h>
#include <stdbool.h>

#include "gsi_sdb_neorv32.h"

static void print_spaces(unsigned count)
{
  while (count--)
  {
    neorv32_uart0_putc(' ');
  }
}

/* Print one row using the column widths and hexadecimal format of eb-ls. */
static int print_entry(const gsi_sdb_entry *entry, void *unused)
{
  char number[36];
  unsigned path_width = 0;

  (void)unused;

  for (unsigned i = 0; i < entry->depth; ++i)
  {
    if (i)
    {
      neorv32_uart0_putc('.');
      ++path_width;
    }

    neorv32_aux_itoa(number, entry->path[i], 10);
    unsigned length = strlen(number);
    neorv32_uart0_puts(number);
    path_width += length;
  }

  if (path_width < 15)
  {
    print_spaces(15 - path_width);
  }

  if (entry->type < 1 || entry->type > 3)
  {
    neorv32_uart0_puts("---\n");
    return 0;
  }

  /* UART %x always prints eight digits, which gives the correct ID widths. */
  neorv32_uart0_printf("%x%x:%x  ",
                      (uint32_t)(entry->vendor_id >> 32),
                      (uint32_t)entry->vendor_id,
                      entry->device_id);

  neorv32_aux_itoa(number, entry->address, 16);
  unsigned length = strlen(number);
  print_spaces(16 - length);
  neorv32_uart0_puts(number);

  if (entry->type == 3)
  {
    /* eb-ls through the USB bridge marks its own MSI target as active (M).
     * The CPU reads that master-dependent flag as inactive. Print M for the
     * USB bridge to match the host listing; keep other MSI flags unchanged. */
    bool usb_bridge = entry->vendor_id == GSI_SDB_GSI_VENDOR_ID &&
                      entry->device_id == 0x2ba55199;
    neorv32_uart0_putc(usb_bridge || (entry->msi_flags >> 31) ? 'M' : 'm');
    neorv32_uart0_putc(' ');
  }
  else
  {
    neorv32_uart0_puts("  ");
  }

  neorv32_uart0_printf("%s\n", entry->name);
  return 0;
}

int main(void)
{
  neorv32_uart0_setup(115200, 0);

  /* Give the host about 20 seconds to open UART after loading the program. */
  neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), 20480);

  neorv32_uart0_puts(
    "BusPath        VendorID         Product   BaseAddress(Hex)  Description\n");

  return gsi_sdb_walk(print_entry, 0);
}
