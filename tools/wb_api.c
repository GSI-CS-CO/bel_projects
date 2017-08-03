/*********************************************************************************************
 * wb_api.cpp
 *
 *  created : Apr 10, 2013
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 03-Aug-2017
 *
 * Api for wishbone devices for timing receiver nodes. This is not a timing receiver API,
 * but only a temporary solution.
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http: *www.gnu.org/licenses/>.
 *
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 25-April-2013
 ********************************************************************************************/

/* standard includes */
#include <string.h>
#include <stdio.h>

/* etherbone */
#include <etherbone.h>

/* 1-wire */
#include <w1.h>

/* wishbone api */
#include <wb_api.h>
#include <wb_slaves.h>

/* global variables */
eb_device_t  known_dev      = EB_NULL;   /* etherbone device */
eb_socket_t  known_sock     = EB_NULL;   /* etherbone socket */
eb_address_t disp_addr      = EB_NULL;   /* wishbone device base address */
eb_address_t pps_addr       = EB_NULL;
eb_address_t eca_addr       = EB_NULL;
eb_address_t endpoint_addr  = EB_NULL;
eb_address_t etherbone_addr = EB_NULL;
eb_address_t tlu_addr       = EB_NULL;
eb_address_t wb4_ram        = EB_NULL;
eb_address_t wb4_1wire      = EB_NULL;

eb_address_t BASE_ONEWIRE;
extern struct w1_bus wrpc_w1_bus;



/* private routines */
static void wb_warn(eb_status_t status, const char* what) {
  fprintf(stderr, "wb_api: warn %s: %s\n", what, eb_status(status));
}

static eb_status_t wb_check_device(eb_device_t device, uint64_t vendor_id, uint32_t product_id, uint8_t ver_major, uint8_t ver_minor, int devIndex, eb_address_t *addr)
{
  eb_address_t tmp;
  eb_status_t  status;

  if ((known_dev == EB_NULL) || (known_dev != device) ||  (*addr == EB_NULL)) {
    known_dev = EB_NULL;
    *addr     = EB_NULL;
  }

  if ((status = wb_get_device_address(device, vendor_id, product_id, ver_major, ver_minor, devIndex, &tmp)) != EB_OK) return status;

  known_dev = device;
  *addr     = tmp;

  return status;
} /* wb_check device */

/* public routines */
eb_status_t wb_open(const char *dev, eb_device_t *device, eb_socket_t *socket)
{
  eb_status_t status;

#ifdef WB_SIMULATE
  *device = EB_NULL;
  *socket = EB_NULL;

  return EB_OK;
#endif

  *device = EB_NULL;
  *socket = EB_NULL;

  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDRX|EB_DATAX, socket)) != EB_OK) return status;
  if ((status = eb_device_open(*socket, dev, EB_ADDRX|EB_DATAX, 3, device)) != EB_OK) return status;

  known_sock = *socket;

  return status;
} /* wb_open */

void wb_close(eb_device_t device, eb_socket_t socket)
{
#ifdef WB_SIMULATE
  return;
#endif

  eb_device_close(device);
  eb_socket_close(socket);
} /* wb_close */


eb_status_t wb_get_device_address(eb_device_t device, uint64_t vendor_id, uint32_t product_id, uint8_t ver_major, uint8_t ver_minor, int devIndex, eb_address_t *address)
{
  eb_status_t       status;
  int               maxDev = 10;
  struct sdb_device sdbDevice[maxDev];
  int               nDevices;
  char              buff[1024];


  if (devIndex >= maxDev) {
    sprintf(buff, "device vendor %"PRIx64", product %x : devIndex exceeds maximum %d (need to change wb_api)", vendor_id, product_id, maxDev);
    wb_warn(EB_OOM, buff);
    return EB_OOM;
  }

  if (devIndex < 0) {
    sprintf(buff, "device vendor %"PRIx64", product %x : devIndex must be larger than 0", vendor_id, product_id);
    wb_warn(EB_OOM, buff);
    return EB_OOM;
  }

#ifdef WB_SIMULATE
  *address = 0;

  return EB_OK;
#endif

  *address = EB_NULL;
  nDevices = maxDev;

  if ((status = eb_sdb_find_by_identity(device, vendor_id, product_id, sdbDevice, &nDevices)) != EB_OK) return status;
  if (nDevices == 0) {
    sprintf(buff, "device vendor %"PRIx64", product %x does not exist!", vendor_id, product_id);
    wb_warn(EB_FAIL, buff);
    return EB_FAIL;
  }
  if (nDevices > maxDev) {
    sprintf(buff, "device vendor %"PRIx64", product %x : too many devices (need to change wb_api)!", vendor_id, product_id);
    wb_warn(EB_OOM, buff);
    return EB_OOM;
  }
  if (nDevices < devIndex + 1) {
    sprintf(buff, "device vendor %"PRIx64", product %x, requested wishbone device does not exist on the bus!", vendor_id, product_id);
    wb_warn(EB_OOM, buff);
    return EB_OOM;
  }
  if (sdbDevice[devIndex].abi_ver_major != ver_major) {
    sprintf(buff, "device vendor %"PRIx64", product %x : major version == %d expected", vendor_id, product_id, ver_major);
    wb_warn(EB_ABI, buff);
    return EB_ABI;
  }
  if (sdbDevice[devIndex].abi_ver_minor <  ver_minor) {
    sprintf(buff, "device vendor %"PRIx64", product %x : minor version >= %d expected", vendor_id, product_id, ver_minor);
    wb_warn(EB_ABI, buff);
    return EB_ABI;
  }


  *address = sdbDevice[devIndex].sdb_component.addr_first;

  return status;
} /* wb_get_device_address */

eb_status_t wb_wr_get_time(eb_device_t device, int devIndex, uint64_t *nsecs)
{
  eb_data_t    data1;
  eb_data_t    data2;
  eb_data_t    data3;
  eb_status_t  status;
  eb_cycle_t   cycle;

#ifdef WB_SIMULATE
  *nsecs       = 1000000123456789;

  return EB_OK;
#endif

  /* get time from ECA */
  *nsecs = 0;
  if ((status = wb_check_device(device, ECA_CTRL_VENDOR, ECA_CTRL_PRODUCT, ECA_CTRL_VMAJOR, ECA_CTRL_VMINOR, devIndex, &eca_addr)) != EB_OK) return status;
  do {
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
    eb_cycle_read(cycle, eca_addr + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &data1);
    eb_cycle_read(cycle, eca_addr + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &data2);
    eb_cycle_read(cycle, eca_addr + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &data3);
    if ((status = eb_cycle_close(cycle)) != EB_OK) return status;
  } while (data1 != data3);

  /* time */
  *nsecs = (uint64_t)data1 << 32;
  *nsecs = *nsecs + (uint64_t)data2;

  return (status);
} /* wb_wr_get_time */


eb_status_t wb_wr_get_mac(eb_device_t device, int devIndex, uint64_t *mac )
{
  eb_data_t    hidata;
  eb_data_t    lodata;
  eb_address_t address;
  eb_status_t  status;

#ifdef WB_SIMULATE
  *mac = 0x0000c5c012345678;

  return EB_OK;
#endif

  *mac = 0x0;
  if ((status = wb_check_device(device, WR_ENDPOINT_VENDOR, WR_ENDPOINT_PRODUCT, WR_ENDPOINT_VMAJOR, WR_ENDPOINT_VMINOR, devIndex, &endpoint_addr)) != EB_OK) return status;

  address = endpoint_addr + WR_ENDPOINT_MACHI;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &hidata, 0, eb_block)) != EB_OK) return status;
  hidata = WR_ENDPOINT_MACHI_MASK & hidata; /* mask highest bytes */

  address = endpoint_addr + WR_ENDPOINT_MACLO;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &lodata, 0, eb_block)) != EB_OK) return status;

  *mac = hidata;
  *mac = (*mac << 32);
  *mac = *mac + lodata;

  return status;
} /* wb_wr_get_mac */


eb_status_t wb_wr_get_link(eb_device_t device, int devIndex, int *link )
{
  eb_data_t    data;
  eb_address_t address;
  eb_status_t  status;

#ifdef WB_SIMULATE
  *link = 0x1;

  return EB_OK;
#endif

  *link = 0x0;
  if ((status = wb_check_device(device, WR_ENDPOINT_VENDOR, WR_ENDPOINT_PRODUCT, WR_ENDPOINT_VMAJOR, WR_ENDPOINT_VMINOR, devIndex, &endpoint_addr)) != EB_OK) return status;

  address = endpoint_addr + WR_ENDPOINT_LINK;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK) return status;
  data = WR_ENDPOINT_LINK_MASK & data; /* mask highest bytes */
  *link = (data == WR_ENDPOINT_LINK_MASK);

  return status;\
} /* wb_wr_get_link */


eb_status_t wb_wr_get_ip(eb_device_t device, int devIndex, int *ip )
{
  eb_data_t    data;
  eb_address_t address;
  eb_status_t  status;

#ifdef WB_SIMULATE
  *ip = 0x1234abcd;

  return EB_OK;
#endif

  *ip = 0x0;
  if ((status = wb_check_device(device, ETHERBONE_CONFIG_VENDOR, ETHERBONE_CONFIG_PRODUCT, ETHERBONE_CONFIG_VMAJOR, ETHERBONE_CONFIG_VMINOR, devIndex, &etherbone_addr)) != EB_OK) return status;

  address = etherbone_addr + ETHERBONE_CONFIG_IP;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK) return status;
  *ip = data;

  return status;
} /* wb_wr_get_ip */


eb_status_t wb_wr_get_sync_state(eb_device_t device, int devIndex, int *syncState )
{
  eb_address_t address;
  eb_data_t    data;
  eb_status_t  status;

#ifdef WB_SIMULATE
  *syncState  = WR_PPS_GEN_ESCR_MASK + 0x8;

  return EB_OK;
#endif

  *syncState  = 0x0;

  if ((status = wb_check_device(device, WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT, WR_PPS_GEN_VMAJOR, WR_PPS_GEN_VMINOR, devIndex, &pps_addr)) != EB_OK) return status;

  address = pps_addr + WR_PPS_GEN_ESCR;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK) return status;
  *syncState  = data & WR_PPS_GEN_ESCR_MASK; /* need to mask relevant bits */

  return status;
} /* wb_wr_get_sync_state */


eb_status_t wb_wr_get_id(eb_device_t device, int devIndex, unsigned int busIndex, unsigned int family, uint64_t *id)
{
  eb_status_t  status;
  uint64_t     oneWireID;

  struct       w1_dev *d;
  int          i;


#ifdef WB_SIMULATE
  *id   = 0x1234567890ABCDEF;

  return EB_OK;
#endif

  *id   = 0x0;

  if ((status = wb_check_device(device, WB4_PERIPH_1WIRE_VENDOR, WB4_PERIPH_1WIRE_PRODUCT, WB4_PERIPH_1WIRE_VMAJOR, WB4_PERIPH_1WIRE_VMINOR, devIndex, &wb4_1wire)) != EB_OK) return status;

  BASE_ONEWIRE = wb4_1wire;

  wrpc_w1_bus.detail = busIndex;
  wrpc_w1_init();
  w1_scan_bus(&wrpc_w1_bus);
  for (i = 0; i < W1_MAX_DEVICES; i++) {
    d = wrpc_w1_bus.devs + i;
    if ((d->rom & 0xff) == family) {
      oneWireID = (int)(d->rom >> 32);
      oneWireID = (oneWireID << 32);
      oneWireID = oneWireID + (int)(d->rom);
      *id = oneWireID;
      return status;
    } /* if d->rom ... */
  } /* for i */
  
  return EB_OOM;
} /* wb_wr_get_id */


eb_status_t wb_wr_get_temp(eb_device_t device, int devIndex, unsigned int busIndex, unsigned int family, double *temp)
{
  eb_status_t  status;
  int          tmpT;

  struct       w1_dev *d;
  int          i;


#ifdef WB_SIMULATE
  *temp   = 0x22.2222;

  return EB_OK;
#endif

  *temp   = 0.0;

  if ((status = wb_check_device(device, WB4_PERIPH_1WIRE_VENDOR, WB4_PERIPH_1WIRE_PRODUCT, WB4_PERIPH_1WIRE_VMAJOR, WB4_PERIPH_1WIRE_VMINOR, devIndex, &wb4_1wire)) != EB_OK) return status;

  BASE_ONEWIRE = wb4_1wire;
  wrpc_w1_bus.detail = busIndex;

  wrpc_w1_init();
  w1_scan_bus(&wrpc_w1_bus);

  for (i = 0; i < W1_MAX_DEVICES; i++) {
    d = wrpc_w1_bus.devs + i;
    if ((d->rom & 0xff) == family) {
      tmpT = w1_read_temp(wrpc_w1_bus.devs + i, 0);
      *temp = (tmpT >> 16) + ((int)((tmpT & 0xffff) * 10 * 1000 >> 16))/10000.0;
      return status;
    } /* if d->rom ... */
  } /* for i */
  
  return EB_OOM; /* no 1-wire temperature sensor at specified WB device and specified 1-wire bus ... */
} /* wb_wr_get_temp */
