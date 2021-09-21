//
// eb_demo: quick hack demo for reading the WR time
//
// this example reads the WR time from the ECA which is
// faster than accessing the White Rabbit core
// 

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Etherbone
#include <etherbone.h>

// include header from the ECA itself to be sure we are consistent with
// the VHDL of this git branch
//#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"
// ok! this example is self-consistent, so lets define the
// the relevant constants here. THIS IS NOT CLEAN!
#define ECA_SDB_VENDOR_ID   0x00000651    // vendor ID (0x651 'GSI')
#define ECA_SDB_DEVICE_ID   0xb2afc251    // device ID 
#define ECA_TIME_HI_GET     0x18          //ro, 32 b, Ticks (nanoseconds) since Jan 1, 1970 (high word)
#define ECA_TIME_LO_GET     0x1c          //ro, 32 b, Ticks (nanoseconds) since Jan 1, 1970 (low word)
#define ECA_SDB_VMAJOR      1             // major revision
#define ECA_SDB_VMINOR      0             // minor revision


static const char* program;

void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
    program, where, eb_status(status));
  exit(1);
}

int main(int argc, const char** argv) {
  eb_status_t       status;               // Etherbone status
  eb_device_t 	    device;               // Etherbone device
  eb_socket_t 	    socket;               // Etherbone socket
  eb_cycle_t  	    cycle;                // Etherbone device
  struct sdb_device sdbDevice;            // required for probe of self-describing Wishbone bus
  int               nDevices;             // required for probe of Wishbone bus
  eb_address_t 	    wrEcaCtrl;            // Wishbone address of Wishbone slave 'ECA_UNIT:CONTROL'
  eb_address_t      taiLo;                // low 32 bit of White Rabbit time
  eb_address_t      taiHi;                // hith 32 bit of White Rabbit time
  eb_data_t         data1, data2, data3;  // Etherbone data
  const char*       devName;              // Etherbone device name

  uint64_t          t;                    // White Rabbit time

  uint64_t          ns;                   // required for convenient display of time
  char time[60];                          
  time_t secs;
  const struct tm* tm;
  
  program = argv[0];
  if (argc < 2) {
    fprintf(stderr, "Syntax: %s <protocol/host/port>\n", argv[0]);
    return 1;
  }

  program = argv[0];
  devName = argv[1];
  
  // Open a socket supporting only 32-bit operations.
  // As we are not exporting any slaves, we don't care what port we get => 0.
  // This function always returns immediately.
  // EB_ABI_CODE helps detect if the application matches the library.
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open", status);
  
  // Open the remote device with 3 attempts to negotiate bus width.
  // This function is blocking and may stall the thread for up to 3 seconds.
  // If you need asynchronous open, see eb_device_open_nb.
  // Note: the supported widths can never be more than the socket supports.
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open", status);
  
  // Find the ECA_CTRL device on the remote Wishbone bus using the SDB records.
  // Blocking call; use eb_sdb_scan_* for asynchronous access to full SDB table.
  // Increase sdbDevice and initial nDevices value to support multiple results,
  // as there might be multiple instances of the same type on the bus.
  // nDevices reports the number of devices found (potentially more than one fits).
  // Important: When calling eb_sdb_find... set the nDevices to the maximum number
  // of devices you like to search for. 
  nDevices = 1;                           // we just have one ECA ;-) 

  if ((status = eb_sdb_find_by_identity(device, ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID, &sdbDevice, &nDevices)) != EB_OK)
    die("ECA_CTRL eb_sdb_find_by_identity", status);
  
  // check if a unique Wishbone device exists
  if (nDevices != 1)
    die("no ECA_CTRL gen found", EB_FAIL);
  // check version of Wishbone device 
  if (ECA_SDB_VMAJOR != sdbDevice.abi_ver_major)
    die("ECA_CTRL major version conflicting - interface changed:", EB_ABI);
  if (ECA_SDB_VMINOR > sdbDevice.abi_ver_minor)
    die("ECA_CTRL  minor version too old - required features might be missing:", EB_ABI);
  // record the address of the device
  wrEcaCtrl = sdbDevice.sdb_component.addr_first;

  // up to now, a) we have a connection to the (remote) Wishbone bus via Etherbone
  // b) obtained the address of the Wishbone device and c) checked its version number
  // this should only be done ONCE, when starting the application. From
  // now on, we talk to our Wishbone slave directly.

  // example how to read data from a Wishbone device during run-time
  // calculate register addresses
  taiLo = wrEcaCtrl +  ECA_TIME_LO_GET;
  taiHi = wrEcaCtrl +  ECA_TIME_HI_GET;

  // we must make sure the high word does not change while reading the low word: use a loop
  do {
    // open Etherbone cycle
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("can't open cycle:", status);
    // read data
    eb_cycle_read(cycle, taiHi, EB_BIG_ENDIAN|EB_DATA32, &data1);
    eb_cycle_read(cycle, taiLo, EB_BIG_ENDIAN|EB_DATA32, &data2);
    eb_cycle_read(cycle, taiHi, EB_BIG_ENDIAN|EB_DATA32, &data3);
    // close cycle
    if ((status = eb_cycle_close(cycle)) != EB_OK) die("can't close cycle:", status);
  } while (data1 != data3);

  t  = (uint64_t)data1 << 32;
  t += (uint64_t)data2;

  // format data and time
  ns    = t % 1000000000;
  secs  = t / 1000000000;
  
  tm = localtime(&secs);
  strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", tm);

  // print the result 
  printf("Current TAI      : %s.%09lu s\n", time, ns);

  // close handler cleanly 
  if ((status = eb_device_close(device)) != EB_OK) die("eb_device_close", status);
  if ((status = eb_socket_close(socket)) != EB_OK) die("eb_socket_close", status);
  
  return 0;
}
