//////////////////////////////////////////////////////////////////////////////////////////////
// wb_api.cpp
//
//  created : Apr 10, 2013
//  author  : Dietrich Beck, GSI-Darmstadt
//  version : 26-Sep-2019
//
// Api for wishbone devices for timing receiver nodes. This is not a timing receiver API,
// but only a temporary solution.
//
// -------------------------------------------------------------------------------------------
// License Agreement for this software:
//
// Copyright (C) 2013  Dietrich Beck
// GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
// Planckstrasse 1
// D-64291 Darmstadt
// Germany
//
// Contact: d.beck@gsi.de
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http: /www.gnu.org/licenses/>.
//
// For all questions and ideas contact: d.beck@gsi.de
// Last update: 25-April-2013
//////////////////////////////////////////////////////////////////////////////////////////////

// standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// etherbone
#include <etherbone.h>

// 1-wire
#include <w1.h>

// wishbone api
#include <wb_api.h>
#include <wb_slaves.h>

// global variables
eb_device_t  known_dev      = EB_NULL;   // etherbone device
eb_socket_t  known_sock     = EB_NULL;   // etherbone socket
eb_address_t disp_addr      = EB_NULL;   // wishbone device base address
eb_address_t pps_addr       = EB_NULL;
eb_address_t eca_addr       = EB_NULL;
eb_address_t endpoint_addr  = EB_NULL;
eb_address_t etherbone_addr = EB_NULL;
eb_address_t tlu_addr       = EB_NULL;
eb_address_t wb4_ram        = EB_NULL;
eb_address_t wb4_1wire      = EB_NULL;
eb_address_t user_1wire     = EB_NULL;
eb_address_t reset_addr     = EB_NULL;
eb_address_t brom_addr      = EB_NULL;
eb_address_t dm_diag_addr   = EB_NULL;
eb_address_t eca_tap_addr   = EB_NULL;

eb_address_t BASE_ONEWIRE;
extern struct w1_bus wrpc_w1_bus;



// private routines
static void wb_warn(eb_status_t status, const char* what) {
  fprintf(stderr, "wb_api: warn %s: %s\n", what, eb_status(status));
}

static eb_status_t wb_check_device(eb_device_t device, uint64_t vendor_id, uint32_t product_id, uint8_t ver_major, uint8_t ver_minor, int devIndex, eb_address_t *addr)
{
  eb_address_t tmp;
  eb_status_t  status;
  int          nDevices;

  if ((known_dev == EB_NULL) || (known_dev != device) ||  (*addr == EB_NULL)) {
    known_dev = EB_NULL;
    *addr     = EB_NULL;
  }

  if ((status = wb_get_device_address(device, vendor_id, product_id, ver_major, ver_minor, devIndex, &tmp, &nDevices)) != EB_OK) return status;

  known_dev = device;
  *addr     = tmp;

  return status;
} // wb_check device


// following CRC routine inspired by
// - http://github.com/paeaetech/paeae/tree/master/Libraries/ds2482/
// - GPL, Paeae Technologies
// - the 1-Wire CRC scheme is described in Maxim Application Note 27:

uint8_t wire1_crc8(uint8_t *addr, uint8_t len, uint8_t family )
{
  uint8_t crc=0;
  uint8_t i,j;
  uint8_t inbyte;
  uint8_t mix;

  if (family == 0x28) {    // this implements a hackish solution for family 0x28; let me know if you have a good idea
    crc = addr[len];       // len as index is ok, len is number of least significant bytes the CRC is calculated on. For 8 byte data, the highest byte containes CRC and len == 7
    return crc;
  } // family 28
  
  for (i=0; i<len;i++) {
    inbyte = addr[i];
    for (j=0;j<8;j++) {
      mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      
      inbyte >>= 1;
    } // for j
  } // for i
  return crc;
} // wire1_crc8

// public routines
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
  if ((status = eb_device_open(*socket, dev, EB_ADDRX|EB_DATAX, 2, device)) != EB_OK) return status;

  known_sock = *socket;

  return status;
} // wb_open

eb_status_t wb_close(eb_device_t device, eb_socket_t socket)
{
  eb_status_t status;

#ifdef WB_SIMULATE
  return EB_OK;
#endif

  if ((status = eb_device_close(device)) != EB_OK) return status;
  if ((status = eb_socket_close(socket)) != EB_OK) return status;

  known_sock   = EB_NULL;
  known_dev    = EB_NULL;

  return status;
} // wb_close


eb_status_t wb_get_device_address(eb_device_t device, uint64_t vendor_id, uint32_t product_id, uint8_t ver_major, uint8_t ver_minor, int devIndex, eb_address_t *address, int *nDevices)
{
  eb_status_t       status;
  int               maxDev = 16;
  struct sdb_device sdbDevice[maxDev];
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
  *nDevices = maxDev;

  if ((status = eb_sdb_find_by_identity(device, vendor_id, product_id, sdbDevice, nDevices)) != EB_OK) return status;
  if (*nDevices == 0) {
    sprintf(buff, "device vendor %"PRIx64", product %x does not exist!", vendor_id, product_id);
    wb_warn(EB_FAIL, buff);
    return EB_FAIL;
  }
  if (*nDevices > maxDev) {
    sprintf(buff, "device vendor %"PRIx64", product %x : too many devices (need to change wb_api)!", vendor_id, product_id);
    wb_warn(EB_OOM, buff);
    return EB_OOM;
  }
  if (*nDevices < devIndex + 1) {
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
} // wb_get_device_address

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

  // get time from ECA 
  *nsecs = 0;
  if ((status = wb_check_device(device, ECA_CTRL_VENDOR, ECA_CTRL_PRODUCT, ECA_CTRL_VMAJOR, ECA_CTRL_VMINOR, devIndex, &eca_addr)) != EB_OK) return status;
  do {
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
    eb_cycle_read(cycle, eca_addr + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &data1);
    eb_cycle_read(cycle, eca_addr + ECA_CTRL_TIME_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &data2);
    eb_cycle_read(cycle, eca_addr + ECA_CTRL_TIME_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &data3);
    if ((status = eb_cycle_close(cycle)) != EB_OK) return status;
  } while (data1 != data3);

  // time 
  *nsecs = (uint64_t)data1 << 32;
  *nsecs = *nsecs + (uint64_t)data2;

  return (status);
} // wb_wr_get_time 


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
  hidata = WR_ENDPOINT_MACHI_MASK & hidata; // mask highest bytes 

  address = endpoint_addr + WR_ENDPOINT_MACLO;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &lodata, 0, eb_block)) != EB_OK) return status;

  *mac = hidata;
  *mac = (*mac << 32);
  *mac = *mac + lodata;

  return status;
} // wb_wr_get_mac 


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
} // wb_wr_get_link 


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
} // wb_wr_get_ip 


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
  *syncState  = data & WR_PPS_GEN_ESCR_MASK; // need to mask relevant bits 

  return status;
} // wb_wr_get_sync_state 


eb_status_t wb_wr_get_uptime(eb_device_t device, int devIndex, uint32_t *uptime)
{
  eb_address_t address;
  eb_data_t    data;
  eb_status_t  status;

#ifdef WB_SIMULATE
  *uptime = 4711;

  return EB_OK;
#endif

  *uptime  = 0;

  if ((status = wb_check_device(device, WB4_BLOCKRAM_VENDOR, WB4_BLOCKRAM_PRODUCT, WB4_BLOCKRAM_VMAJOR, WB4_BLOCKRAM_VMINOR, devIndex, &wb4_ram)) != EB_OK) return status;

  address = wb4_ram + WB4_BLOCKRAM_WR_UPTIME;
  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK) return status;
  *uptime = data;

  return status;
} // wb_wr_get_uptime


eb_status_t wb_wr_stats_get_lock(eb_device_t device, int devIndex, uint64_t *lockLossTS, uint64_t *lockAcqTS, uint32_t *lockNAcq)
{
  eb_cycle_t   cycle;
  eb_data_t    data0, data1, data2, data3, data4;
  eb_status_t  status;

  int          syncState;

#ifdef WB_SIMULATE
  *lockLossTS = 0xffffffffffffffff;
  *lockAcqTS  = 4711;
  *lockNAcq   = 17

  return EB_OK;
#endif

  *lockLossTS = 0xffffffffffffffff;
  *lockAcqTS  = 0xffffffffffffffff;
  *lockNAcq   = 0xffffffff;

  if ((status = wb_check_device(device, DM_DIAG_VENDOR, DM_DIAG_PRODUCT, DM_DIAG_VMAJOR, DM_DIAG_VMINOR, devIndex, &dm_diag_addr)) != EB_OK) return status;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_WR_LOCK_LOSS_LAST_TS_GET_0, EB_BIG_ENDIAN|EB_DATA32, &data0);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_WR_LOCK_LOSS_LAST_TS_GET_1, EB_BIG_ENDIAN|EB_DATA32, &data1);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_WR_LOCK_ACQU_LAST_TS_GET_0, EB_BIG_ENDIAN|EB_DATA32, &data2);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_WR_LOCK_ACQU_LAST_TS_GET_1, EB_BIG_ENDIAN|EB_DATA32, &data3);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_WR_LOCK_CNT_GET_0,          EB_BIG_ENDIAN|EB_DATA32, &data4);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;

  *lockLossTS = (uint64_t)data1 << 32;
  *lockLossTS = *lockLossTS + (uint64_t)data0;
  *lockAcqTS  = (uint64_t)data3 << 32;
  *lockAcqTS  = *lockAcqTS  + (uint64_t)data2;
  *lockNAcq   = data4;

  // mark nsecs in case no lock has been acquired so far
  if (*lockNAcq == 0) {
    *lockLossTS = 0xffffffffffffffff;
    *lockAcqTS  = 0xffffffffffffffff;
  }

  // mark nsecsAcq in case TR is not in TRACK_PHASE
  wb_wr_get_sync_state(device, 0, &syncState);
  if (syncState != WR_PPS_GEN_ESCR_MASK) {
    *lockAcqTS  = 0xffffffffffffffff;
  }

  return status;
} // wb_wr_get_lock_stats  


eb_status_t wb_wr_stats_get_continuity(eb_device_t device, int devIndex, uint64_t *contObsT, int64_t  *contMaxPosDT, uint64_t *contMaxPosTS, int64_t  *contMaxNegDT, uint64_t *contMaxNegTS)
{ 
  eb_cycle_t   cycle;
  eb_data_t    data0, data1, data2, data3, data4, data5, data6, data7, data8, data9;
  eb_status_t  status;


#ifdef WB_SIMULATE
  *contObsT     = 0x64
  *contMaxPosDT = 0x32;
  *contMaxPosTS = 0x4711;
  *contMaxNetDT = 0x7fffffffffff4711;
  *contMaxNegTS = 0x4712;

  return EB_OK;
#endif

  *contObsT     = 0xffffffffffffffff;
  *contMaxPosDT = 0xffffffffffffffff;
  *contMaxPosTS = 0xffffffffffffffff;
  *contMaxNegDT = 0xffffffffffffffff;
  *contMaxNegTS = 0xffffffffffffffff;

  if ((status = wb_check_device(device, DM_DIAG_VENDOR, DM_DIAG_PRODUCT, DM_DIAG_VMAJOR, DM_DIAG_VMINOR, devIndex, &dm_diag_addr)) != EB_OK) return status;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_0, EB_BIG_ENDIAN|EB_DATA32, &data0);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_1, EB_BIG_ENDIAN|EB_DATA32, &data1);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_POS_GET_0,             EB_BIG_ENDIAN|EB_DATA32, &data2);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_POS_GET_1,             EB_BIG_ENDIAN|EB_DATA32, &data3);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_POS_TS_GET_0,          EB_BIG_ENDIAN|EB_DATA32, &data4);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_POS_TS_GET_1,          EB_BIG_ENDIAN|EB_DATA32, &data5);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_NEG_GET_0,             EB_BIG_ENDIAN|EB_DATA32, &data6);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_NEG_GET_1,             EB_BIG_ENDIAN|EB_DATA32, &data7);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_NEG_TS_GET_0,          EB_BIG_ENDIAN|EB_DATA32, &data8);
  eb_cycle_read(cycle, dm_diag_addr + DM_DIAG_TIME_DIF_NEG_TS_GET_1,          EB_BIG_ENDIAN|EB_DATA32, &data9);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;

  *contObsT     = (uint64_t)data1 << 32;
  *contObsT     = *contObsT + (uint64_t)data0;
  *contMaxPosDT = (uint64_t)data3 << 32;
  *contMaxPosDT = *contMaxPosDT + (uint64_t)data2;
  *contMaxPosTS = (uint64_t)data5 << 32;
  *contMaxPosTS = *contMaxPosTS + (uint64_t)data4;
  *contMaxNegDT = (uint64_t)data7 << 32;
  *contMaxNegDT = *contMaxNegDT + (uint64_t)data6;
  *contMaxNegTS = (uint64_t)data9 << 32;
  *contMaxNegTS = *contMaxNegTS + (uint64_t)data8;

  return status;
} // wb_wr_stats_get_continuity


eb_status_t wb_wr_stats_get_stall(eb_device_t device, int devIndex, uint32_t stallObsCPU, uint64_t *stallObsT, uint32_t *stallMaxStreak, uint32_t *stallN, uint64_t *stallTS)
{
  eb_cycle_t   cycle;
  eb_data_t    data0, data1, data2, data3, data4;
  eb_status_t  status;


#ifdef WB_SIMULATE
  *stallObsT      = 0x64;
  *stallMaxStreak = 0x17;
  *stallN         = 0x42;
  *stallTS        = 0x4711;

  return EB_OK;
#endif

  *stallObsT      = 0xffffffffffffffff;
  *stallMaxStreak = 0xffffffff; 
  *stallN         = 0xffffffff;
  *stallTS        = 0xffffffffffffffff;

  if ((status = wb_check_device(device, DM_DIAG_VENDOR, DM_DIAG_PRODUCT, DM_DIAG_VMAJOR, DM_DIAG_VMINOR, devIndex, &dm_diag_addr)) != EB_OK) return status;

  // select CPU
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_write(cycle, dm_diag_addr + DM_DIAG_STALL_STAT_SELECT_RW,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)stallObsCPU);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;

  // read data
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_read(cycle , dm_diag_addr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW, EB_BIG_ENDIAN|EB_DATA32, &data0);
  eb_cycle_read(cycle , dm_diag_addr + DM_DIAG_STALL_STREAK_MAX_GET,          EB_BIG_ENDIAN|EB_DATA32, &data1);
  eb_cycle_read(cycle , dm_diag_addr + DM_DIAG_STALL_CNT_GET,                 EB_BIG_ENDIAN|EB_DATA32, &data2);
  eb_cycle_read(cycle , dm_diag_addr + DM_DIAG_STALL_MAX_TS_GET_0,            EB_BIG_ENDIAN|EB_DATA32, &data3);
  eb_cycle_read(cycle , dm_diag_addr + DM_DIAG_STALL_MAX_TS_GET_1,            EB_BIG_ENDIAN|EB_DATA32, &data4);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;

  *stallObsT      = 0;
  *stallObsT      = *stallObsT + (uint64_t)data0;
  *stallMaxStreak = (uint32_t)data1;
  *stallN         = (uint32_t)data2;
  *stallTS        = (uint64_t)data4 << 32;
  *stallTS        = *stallTS + (uint64_t)data3;
  
  return status;
} // wb_wr_stats_get_stall


eb_status_t wb_wr_stats_reset(eb_device_t device, int devIndex, uint64_t contObsT, uint32_t stallObsT)
{
  eb_cycle_t  cycle;
  uint32_t    contObsTHi, contObsTLo;
  eb_status_t status;

  contObsTHi = (uint32_t)(contObsT >> 32);
  contObsTLo = (uint32_t)(contObsT & 0xffffffff);

  if ((status = wb_check_device(device, DM_DIAG_VENDOR, DM_DIAG_PRODUCT, DM_DIAG_VMAJOR, DM_DIAG_VMINOR, devIndex, &dm_diag_addr)) != EB_OK) return status;
  
  // set intervals
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_write(cycle, dm_diag_addr + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_1, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)contObsTHi);
  eb_cycle_write(cycle, dm_diag_addr + DM_DIAG_TIME_OBSERVATION_INTERVAL_RW_0, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)contObsTLo);
  eb_cycle_write(cycle, dm_diag_addr + DM_DIAG_STALL_OBSERVATION_INTERVAL_RW,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)stallObsT);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;

  // reset
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;  
  eb_cycle_write(cycle, dm_diag_addr + DM_DIAG_RESET_OWR,                      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)0x1);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;
  
  return status;
} // wb_wr_stats_reset


eb_status_t wb_eca_stats_reset(eb_device_t device, int devIndex, int32_t lateOffset)
{
  eb_cycle_t  cycle;
  eb_status_t status;

  if ((status = wb_check_device(device, ECA_TAP_VENDOR, ECA_TAP_PRODUCT, ECA_TAP_VMAJOR, ECA_TAP_VMINOR, devIndex, &eca_tap_addr)) != EB_OK) return status;

  // reset 
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;  
  eb_cycle_write(cycle, eca_tap_addr + ECA_TAP_RESET_OWR, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)0x1);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;


  // clear counters and set late offset
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_write(cycle, eca_tap_addr + ECA_TAP_OFFSET_LATE_RW, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)lateOffset);
  eb_cycle_write(cycle, eca_tap_addr + ECA_TAP_CLEAR_OWR,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)0xf);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;
  
  return status;  
} // wb_eca_stats_reset


eb_status_t wb_eca_stats_clear(eb_device_t device, int devIndex, uint32_t clearFlag)
{
  eb_cycle_t  cycle;
  eb_status_t status;

  if ((status = wb_check_device(device, ECA_TAP_VENDOR, ECA_TAP_PRODUCT, ECA_TAP_VMAJOR, ECA_TAP_VMINOR, devIndex, &eca_tap_addr)) != EB_OK) return status;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;  
  eb_cycle_write(cycle, eca_tap_addr + ECA_TAP_CLEAR_OWR, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)clearFlag);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;
  
  return status;  
} // wb_wr_stats_clear


eb_status_t wb_eca_stats_enable(eb_device_t device, int devIndex, uint32_t enableFlag)
{
  eb_cycle_t  cycle;
  eb_status_t status;

  if ((status = wb_check_device(device, ECA_TAP_VENDOR, ECA_TAP_PRODUCT, ECA_TAP_VMAJOR, ECA_TAP_VMINOR, devIndex, &eca_tap_addr)) != EB_OK) return status;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;  
  eb_cycle_write(cycle, eca_tap_addr + ECA_TAP_CAPTURE_RW, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)enableFlag);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;
  
  return status;  
} // wb_eca_stats_enable


eb_status_t wb_eca_stats_get(eb_device_t device, int devIndex, uint64_t *nMessage, int64_t *dtSum, int64_t *dtMin, int64_t *dtMax, uint32_t *nLate, int32_t *lateOffset)
{
  eb_cycle_t   cycle;
  eb_data_t    data0, data1, data2, data3, data4, data5, data6, data7, data8, data9;
  eb_status_t  status;


#ifdef WB_SIMULATE
  *nMessage     = 17;
  *dtSum        = 0x47114711;
  *dtMin        = 444;
  *dtMax        = 888;

  return EB_OK;
#endif

  *nMessage     = 0xffffffffffffffff;
  *dtSum        = 0xffffffffffffffff;
  *dtMin        = 0xffffffffffffffff;
  *dtMax        = 0xffffffffffffffff;

  if ((status = wb_check_device(device, ECA_TAP_VENDOR, ECA_TAP_PRODUCT, ECA_TAP_VMAJOR, ECA_TAP_VMINOR, devIndex, &eca_tap_addr)) != EB_OK) return status;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_CNT_MSG_GET_0,  EB_BIG_ENDIAN|EB_DATA32, &data0);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_CNT_MSG_GET_1,  EB_BIG_ENDIAN|EB_DATA32, &data1);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_DIFF_ACC_GET_0, EB_BIG_ENDIAN|EB_DATA32, &data2);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_DIFF_ACC_GET_1, EB_BIG_ENDIAN|EB_DATA32, &data3);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_DIFF_MIN_GET_0, EB_BIG_ENDIAN|EB_DATA32, &data4);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_DIFF_MIN_GET_1, EB_BIG_ENDIAN|EB_DATA32, &data5);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_DIFF_MAX_GET_0, EB_BIG_ENDIAN|EB_DATA32, &data6);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_DIFF_MAX_GET_1, EB_BIG_ENDIAN|EB_DATA32, &data7);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_CNT_LATE_GET,   EB_BIG_ENDIAN|EB_DATA32, &data8);
  eb_cycle_read(cycle, eca_tap_addr + ECA_TAP_OFFSET_LATE_RW, EB_BIG_ENDIAN|EB_DATA32, &data9);
  if ((status = eb_cycle_close(cycle)) != EB_OK) return status;

  *nMessage     = (uint64_t)data1 << 32;
  *nMessage     = *nMessage + (uint64_t)data0;
  *dtSum        = (uint64_t)data3 << 32;
  *dtSum        = *dtSum + (uint64_t)data2;
  *dtMin        = (uint64_t)data5 << 32;
  *dtMin        = *dtMin + (uint64_t)data4;
  *dtMax        = (uint64_t)data7 << 32;
  *dtMax        = *dtMax + (uint64_t)data6;
  *nLate        = (uint32_t)data8;
  *lateOffset   = (int32_t)data9;
  
  return status;
} // wb_eca_stats_get


eb_status_t wb_1wire_get_id(eb_device_t device, int devIndex, unsigned int busIndex, unsigned int family, short isUserFlag, uint64_t *id)
{
  eb_status_t  status;
  uint8_t      len4CRC;
  uint16_t     CRC;

  struct       w1_dev *d;
  int          i;


#ifdef WB_SIMULATE
  *id   = 0x1234567890ABCDEF;

  return EB_OK;
#endif

  *id     = 0x0;
  
  len4CRC = 7;    // use 7 least significant bytes to calculate CRC
  CRC     = 0;
  

  if (isUserFlag) {
    if ((status = wb_check_device(device, USER_1WIRE_VENDOR, USER_1WIRE_PRODUCT, USER_1WIRE_VMAJOR, USER_1WIRE_VMINOR, devIndex, &user_1wire)) != EB_OK) return status;
    BASE_ONEWIRE = user_1wire;
  }
  else {
    if ((status = wb_check_device(device, WB4_PERIPH_1WIRE_VENDOR, WB4_PERIPH_1WIRE_PRODUCT, WB4_PERIPH_1WIRE_VMAJOR, WB4_PERIPH_1WIRE_VMINOR, devIndex, &wb4_1wire)) != EB_OK) return status;
    BASE_ONEWIRE = wb4_1wire;
  }

  wrpc_w1_bus.detail = busIndex;
  wrpc_w1_init();
  w1_scan_bus(&wrpc_w1_bus);
  for (i = 0; i < W1_MAX_DEVICES; i++) {
    d = wrpc_w1_bus.devs + i;
    if ((d->rom & 0xff) == family) {
      *id = d->rom;
      CRC = wire1_crc8((uint8_t*)id, len4CRC, family);
      if (!CRC)                     return EB_ADDRESS; // CRC == 0 is illegal
      if (CRC == ((uint8_t*)id)[7]) return status;     // CRC ok
      else                          return EB_ADDRESS; // CRC failed 
    } // if d->rom ... 
  } // for i 
  
  return EB_OOM;
} // wb_1wire_get_id 


eb_status_t wb_1wire_get_temp(eb_device_t device, int devIndex, unsigned int busIndex, unsigned int family, short isUserFlag, double *temp)
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

  if (isUserFlag) {
    if ((status = wb_check_device(device, USER_1WIRE_VENDOR, USER_1WIRE_PRODUCT, USER_1WIRE_VMAJOR, USER_1WIRE_VMINOR, devIndex, &user_1wire)) != EB_OK) return status;
    BASE_ONEWIRE = user_1wire;
  }
  else {
    if ((status = wb_check_device(device, WB4_PERIPH_1WIRE_VENDOR, WB4_PERIPH_1WIRE_PRODUCT, WB4_PERIPH_1WIRE_VMAJOR, WB4_PERIPH_1WIRE_VMINOR, devIndex, &wb4_1wire)) != EB_OK) return status;
    BASE_ONEWIRE = wb4_1wire;
  }

  wrpc_w1_bus.detail = busIndex;

  wrpc_w1_init();
  w1_scan_bus(&wrpc_w1_bus);

  for (i = 0; i < W1_MAX_DEVICES; i++) {
    d = wrpc_w1_bus.devs + i;
    if ((d->rom & 0xff) == family) {
      tmpT = w1_read_temp(wrpc_w1_bus.devs + i, 0);
      *temp = (tmpT >> 16) + ((int)((tmpT & 0xffff) * 10 * 1000 >> 16))/10000.0;
      return status;
    } // if d->rom ... 
  } // for i 
  
  return EB_OOM; // no 1-wire temperature sensor at specified WB device and specified 1-wire bus ... 
} // wb_1wire_get_temp 


eb_status_t wb_wr_reset(eb_device_t device, int devIndex, uint32_t value)
{
  eb_data_t    data;
  eb_address_t address;
  eb_status_t  status;


#ifdef WB_SIMULATE
  return EB_OK;
#endif

  if ((status = wb_check_device(device, FPGA_RESET_VENDOR, FPGA_RESET_PRODUCT, FPGA_RESET_VMAJOR, FPGA_RESET_VMINOR, devIndex, &reset_addr)) != EB_OK) return status;

  address = reset_addr + FPGA_RESET_RESET;
  data    = (eb_data_t)value;

  if ((status = eb_device_write(device, address, EB_BIG_ENDIAN|EB_DATA32, data, 0, eb_block)) != EB_OK) return status;

  return status;
} // wb_wr_reset


eb_status_t wb_wr_watchdog(eb_device_t device, int devIndex)
{
  eb_data_t    data;
  eb_address_t address;
  eb_status_t  status;


#ifdef WB_SIMULATE
  return EB_OK;
#endif

  if ((status = wb_check_device(device, FPGA_RESET_VENDOR, FPGA_RESET_PRODUCT, FPGA_RESET_VMAJOR, FPGA_RESET_VMINOR, devIndex, &reset_addr)) != EB_OK) return status;

  address = reset_addr + FPGA_RESET_WATCHDOG_DISABLE;
  data    = (eb_data_t)0xcafebabe;

  if ((status = eb_device_write(device, address, EB_BIG_ENDIAN|EB_DATA32, data, 0, eb_block)) != EB_OK) return status;

  return status;
} // wb_wr_watchdog


eb_status_t wb_cpu_halt(eb_device_t device, int devIndex, uint32_t value)
{
  eb_data_t    data;
  eb_address_t address, tmpAddr;
  eb_status_t  status;
  int          nLM32;


#ifdef WB_SIMULATE
  return EB_OK;
#endif

  // get number of lm32 CPUs by looking for LM32_RAM_USER
  if ((status = wb_get_device_address(device, LM32_RAM_USER_VENDOR, LM32_RAM_USER_PRODUCT, LM32_RAM_USER_VMAJOR, LM32_RAM_USER_VMINOR, 0, &tmpAddr, &nLM32)) != EB_OK) return status;

  // get address of RESET controller
  if ((status = wb_check_device(device, FPGA_RESET_VENDOR, FPGA_RESET_PRODUCT, FPGA_RESET_VMAJOR, FPGA_RESET_VMINOR, devIndex, &reset_addr)) != EB_OK) return status;
  address = reset_addr + FPGA_RESET_USERLM32_SET;

  // reset individual lm32 or all lm32?
  switch (value) {
  case 0 ... 31 :
    if (value >= nLM32) return EB_OOM; // request CPU does not exist 
    data = (eb_data_t)(1 << value);
    break;
  case 0xff :
    data = (eb_data_t)(~(-(1 << nLM32)));
    break;
  default :
    return (EB_OOM);
  } // switch value
  
  if ((status = eb_device_write(device, address, EB_BIG_ENDIAN|EB_DATA32, data, 0, eb_block)) != EB_OK) return status;

  return status;
} // wb_cpu_halt


eb_status_t wb_cpu_resume(eb_device_t device, int devIndex, uint32_t value)
{
  eb_data_t    data;
  eb_address_t address;
  eb_status_t  status;


#ifdef WB_SIMULATE
  return EB_OK;
#endif

  if ((status = wb_check_device(device, FPGA_RESET_VENDOR, FPGA_RESET_PRODUCT, FPGA_RESET_VMAJOR, FPGA_RESET_VMINOR, devIndex, &reset_addr)) != EB_OK) return status;

  address = reset_addr + FPGA_RESET_USERLM32_CLEAR;
  switch (value) {
  case 0 ... 31 :
    data = (eb_data_t)(1 << value);
    break;
  case 0xff :
    data = (eb_data_t)(0xffffffff);
    break;
  default :
    return (EB_OOM);
  } // switch value
  
  if ((status = eb_device_write(device, address, EB_BIG_ENDIAN|EB_DATA32, data, 0, eb_block)) != EB_OK) return status;

  return status;
} // wb_cpu_resume


eb_status_t wb_cpu_status(eb_device_t device, int devIndex, uint32_t *value)
{
  eb_data_t    data;
  eb_address_t address;
  eb_status_t  status;


#ifdef WB_SIMULATE
  *value = 0x0;
  return EB_OK;
#endif

  if ((status = wb_check_device(device, FPGA_RESET_VENDOR, FPGA_RESET_PRODUCT, FPGA_RESET_VMAJOR, FPGA_RESET_VMINOR, devIndex, &reset_addr)) != EB_OK) return status;

  address = reset_addr + FPGA_RESET_USERLM32_GET;
  
  status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
  *value = (uint32_t)data;
  
  return status;
} // wb_cpu_status


eb_status_t wb_get_build_type(eb_device_t device, int size, char *buildType)
{
  eb_data_t    *data = NULL;
  char         *text = NULL;
  char         *ptr  = NULL;
  eb_status_t  status;
  eb_cycle_t   cycle;
  struct sdb_device sdb;
  int i,j;
  int datalen  = 0;
  int textlen  = 0;
  int devIndex = 1;

#ifdef WB_SIMULATE
  if (size > 3) sprintf(builtType, "N/A");
  return EB_OK;
#endif
  if ((status = eb_sdb_find_by_identity(device, FPGA_BUILDROM_VENDOR, FPGA_BUILDROM_PRODUCT, &sdb, &devIndex)) != EB_OK) return status;
  if (devIndex != 1) return EB_OOM;
  if ((status = eb_cycle_open(device, 0, 0, &cycle)) == EB_OK) {
    datalen = ((sdb.sdb_component.addr_last - sdb.sdb_component.addr_first) + 1) / 4;
    brom_addr = sdb.sdb_component.addr_first;
    if ((data = malloc(datalen * sizeof(eb_data_t))) == 0) return EB_OOM;
    for (i = 0; i < datalen; ++i) eb_cycle_read(cycle, brom_addr + i*4, EB_DATA32|EB_BIG_ENDIAN, &data[i]);
  } // if cycle open
  eb_cycle_close(cycle);
  
  textlen = datalen * 4 * sizeof(eb_data_t);
  if ((text = malloc(textlen)) == 0) return EB_OOM;

  j = 0;
  for (i = 0; i < datalen; i++) {
    text[j] = (char)(data[i] >> 24) & 0xff; j++;
    text[j] = (char)(data[i] >> 16) & 0xff; j++;
    text[j] = (char)(data[i] >>  8) & 0xff; j++;
    text[j] = (char)(data[i]      ) & 0xff; j++;
  }
  text[j+1] = '\0';

  ptr = strstr(text, "Build type  : "); 
  if (ptr != NULL) ptr = &(ptr[strlen("Build type  : ")]);
  //if (ptr != NULL) ptr = strstr(ptr, "-v");
  //if (ptr != NULL) ptr = &(ptr[strlen("-v")]);
  if (ptr != NULL) ptr = strtok(ptr, "\n");
  if ((ptr != NULL) && (strlen(ptr) < size)) sprintf(buildType, "%s", ptr);
  else                                       buildType[0] = '\0';
 
  if (data != NULL) free(data);
  free(text);

  return EB_OK;
  
} // wb_build_type
