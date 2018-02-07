//
// eb_demo: quick hack demo for accessing Wishbone devices using the native Etherbone library
// 

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

//Etherbone
#include <etherbone.h>

//Wishbone devices
#include <wb_slaves.h>

static const char* program;

void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
    program, where, eb_status(status));
  exit(1);
}

int main(int argc, const char** argv) {
  eb_status_t       status;
  eb_device_t 	    device;
  eb_socket_t 	    socket;
  eb_cycle_t  	    cycle;
  struct sdb_device sdbDevice;
  eb_address_t 	    wrPPS;
  eb_address_t 	    wrEndpoint;
  int               nDevices;
  eb_data_t         data, data2;

  const char* devName;
  char mac[20];
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
  
  /* Open a socket supporting only 32-bit operations.
   * As we are not exporting any slaves, we don't care what port we get => 0.
   * This function always returns immediately.
   * EB_ABI_CODE helps detect if the application matches the library.
   */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open", status);
  
  /* Open the remote device with 3 attempts to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open, see eb_device_open_nb.
   * Note: the supported widths can never be more than the socket supports.
   */
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open", status);
  
  /* Find a PPS device on the remote Wishbone bus using the SDB records.
   * Blocking call; use eb_sdb_scan_* for asynchronous access to full SDB table.
   * Increase sdbDevice and initial nDevices value to support multiple results.
   * nDevices reports the number of devices found (potentially more than fit).
   */
  nDevices = 1;
  if ((status = eb_sdb_find_by_identity(device, WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT, &sdbDevice, &nDevices)) != EB_OK)
    die("PPS eb_sdb_find_by_identity", status);
  
  /* check if a unique Wishbone device exists */
  if (nDevices != 1)
    die("no PPS gen found", EB_FAIL);
  /* check version of Wishbone device */
  if (WR_PPS_GEN_VMAJOR != sdbDevice.abi_ver_major)
    die("PPS major version conflicting - interface changed:", EB_ABI);
  if (WR_PPS_GEN_VMINOR > sdbDevice.abi_ver_minor)
    die("PPS minor version too old - required features might be missing:", EB_ABI);
  /* Record the address of the device */
  wrPPS = sdbDevice.sdb_component.addr_first;

  
  nDevices = 1;

  if ((status = eb_sdb_find_by_identity(device, WR_ENDPOINT_VENDOR, WR_ENDPOINT_PRODUCT, &sdbDevice, &nDevices)) != EB_OK)
    die("EP eb_sdb_find_by_identity", EB_FAIL);
  
  /* check if a unique Wishbone device exists */
  if (nDevices != 1)
    die("expect exactly one endpoint", EB_FAIL);
  /* check version of Wishbone device */
  if (WR_ENDPOINT_VMAJOR != sdbDevice.abi_ver_major)
    die("Endpoint major version conflicting - interface changed:", EB_ABI);
  if (WR_ENDPOINT_VMINOR > sdbDevice.abi_ver_minor)
    die("Endpoint minor version too old - required features might be missing:", EB_ABI);
  /* record the address of the device */
  wrEndpoint = sdbDevice.sdb_component.addr_first;

  /* Read timestamp from PPS generator.
   * Passing "eb_block" for the asynchronous callback makes the call blocking.
   * Upon return, the operation has been completed and data is valid.
   * This function also packs only a single operation into a cycle (and packet)!
   * For high-throughput applications, pack multiple operations per cycle and
   * use asynchronous notification to support pipelining multiple cycles.
   */
  if ((status = eb_device_read(device, wrPPS + WR_PPS_GEN_CNTR_UTCLO, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
    die("PPS eb_device_read", status);
  secs = data;

  /* Format the date */
  tm = localtime(&secs);
  strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S %Z", tm);
  
  /* Read the MAC address from the endpoint (two words) in one cycle.
   * This demonstrates how to read multiple values in a single round trip.
   * eb_cycle_open always returns immediately.
   */
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
    die("EP eb_cycle_open", status);
  
  /* Queueing operations to a cycle never fails (EB_OOM is reported later) */
  eb_cycle_read(cycle, wrEndpoint + WR_ENDPOINT_MACHI, EB_BIG_ENDIAN|EB_DATA32, &data);
  eb_cycle_read(cycle, wrEndpoint + WR_ENDPOINT_MACLO, EB_BIG_ENDIAN|EB_DATA32, &data2);
  
  /* Because the cycle was opened with eb_block, this is a blocking call.
   * Upon termination, data and data2 will be valid.
   * For higher performance, use multiple asynchronous cycles in a pipeline.
   */
  if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("EP eb_cycle_close", status);
  
  /* Format the MAC address */
  snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", 
    (int)((data  >>  8) & 0xff), (int)((data  >>  0) & 0xff), 
    (int)((data2 >> 24) & 0xff), (int)((data2 >> 16) & 0xff),
    (int)((data2 >>  8) & 0xff), (int)((data2 >>  0) & 0xff));

  /* Print the result */
  printf("Current TAI      : %s\n", time);
  printf("WR interface MAC : %s\n", mac);

  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close", status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close", status);
  
  return 0;
}
