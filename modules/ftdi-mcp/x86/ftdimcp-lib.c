/********************************************************************************************
 *  ftdimcp-lib.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Oct-2025
 *
 *  x86 routines for MCP4725 connected via FT232H
 * 
 *  see ftdimcp-lib.h for version, license and documentation 
 *
 ********************************************************************************************/

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// ftdi mcp
#include <ftdimcp-lib.h>


// get version of all involved libraries
void ftdimcp_getVersion(uint32_t *ftdimcp_version, uint32_t *ftdi_version, uint32_t *mpsse_version)
{
  *ftdimcp_version = FTDIMCP_LIB_VERSION;
  Ver_libMPSSE(mpsse_version, ftdi_version);
} // ftdimcp_getVersion


// open connection and initialize stuff
FT_STATUS ftdimcp_open(int cIdx, FT_HANDLE *cHandle, int flagDebug)
{
  FT_STATUS ftStatus;         
  uint32_t  nChannels;

  // init library 
  Init_libMPSSE();

  if ((ftStatus = I2C_GetNumChannels(&nChannels)) != FT_OK) {
    if (flagDebug) printf("can't read number of channels, ftStatus is %d\n", ftStatus);
    return ftStatus;
  } // if ftStatus

  if ((nChannels < 1) && flagDebug) {
    printf("no channel found; # of channels is %d\n", nChannels);
    printf("possible reasons\n");
    printf(" - uninitialized EEPROM; please program it using FT_PROG from FTDI\n");
    printf(" - kernel driver ftdi_sio is active; try one of the following\n");
    printf(" -- rmmod ftdi_sio, rmmod usbserial\n");
    printf(" -- echo -n 1-3:1.0 > /sys/bus/usb/drivers/ftdi_sio/unbind (replace with proper ID)\n");
    printf(" - insufficient privileges -> try 'sudo' or 'chmod'\n");
    printf(" - another program using the device already running (only one is supported)?\n");
  } // if nChannels

  if ((ftStatus = I2C_OpenChannel(0, cHandle)) != FT_OK) {
    if (flagDebug) printf("can't open I2C channel\n");
    return ftStatus;
  } // if ftStatus

  return ftStatus;
} // ftdimcp_open


// closes connection
void ftdimcp_close(FT_HANDLE cHandle)
{
  I2C_CloseChannel(cHandle);
  Cleanup_libMPSSE(); 
} // ftdimcp_close


// gets device info
FT_STATUS ftdimcp_info(int cIdx, uint32_t *deviceId, char *deviceSerial, int flagPrint)
{
  FT_STATUS                ftStatus;          // status returned by ftdi library
  FT_DEVICE_LIST_INFO_NODE channelInfo;       // channel info data


  ftStatus = I2C_GetChannelInfo(cIdx, &channelInfo);

    if (ftStatus == FT_OK) {
      *deviceId = channelInfo.ID;
      sprintf(deviceSerial, "%s", channelInfo.SerialNumber);
      if (flagPrint) {
        printf("description: %s\n"  , channelInfo.Description);
        printf("serial     : %s\n"  , channelInfo.SerialNumber);
        printf("locId      : 0x%x\n", channelInfo.LocId);
        printf("ID         : 0x%x\n", channelInfo.ID);
        printf("type       : 0x%x\n", channelInfo.Type);
        printf("flags      : 0x%x\n", channelInfo.Flags);
      } // if flagprint
    } // if ftStatus
    else {
      *deviceId = 0x0;
      sprintf(deviceSerial, "%s", "");
      if (flagPrint) printf("error      : can't read channel info, ftStatus is %d\n", ftStatus);
    } // else ftStatus

  return ftStatus;
} // ftdimcp_info


// initializes device
FT_STATUS ftdimcp_init(FT_HANDLE cHandle)
{
  FT_STATUS     ftStatus;                         // status returned by ftdi library
  ChannelConfig cConfig;                          // channel config data

  cConfig.ClockRate    = I2C_CLOCK_STANDARD_MODE; // STANDARD_MODE, FAST_MODE, FAST_MODE_PLUS, HIGH_SPEED_MODE
  cConfig.LatencyTimer = 1;                       // 2..255
  cConfig.Options      = 0x0;                     // bit0: 3PhaseDataClocking, bit1: loopback, bit2 clock streching, bit3 reserved
  ftStatus = I2C_InitChannel(cHandle, &cConfig);

  return ftStatus;
} // ftdimcp_init


// sets level of comparator
FT_STATUS ftdimcp_setLevel(FT_HANDLE cHandle, uint32_t dacAddr, double dacLevel, int flagEeprom, int flagDebug)
{
  FT_STATUS ftStatus;
  uint32_t  i2cAddr;
  uint32_t  nTx;                   // number of bytes to write
  uint32_t  nTxd;                  // number of bytes that have been written
  uint8_t   data[10];              // data used for transfer
  uint32_t  transOpt;              // options used for transfer
  uint16_t  dacMax = 0xfff;        // 12 bit DAC
  uint8_t   cmdByte;               // command byte
  uint16_t  dacRaw;                // raw DAC value

  // range checking
  if ((dacLevel > 100.0) || (dacLevel < 0.0)) return FT_INVALID_PARAMETER;

  // conversion [%] -> raw value
  dacRaw   = round(dacLevel * (double)dacMax / 100.0);

  transOpt = 0x03;

  //printf("dac raw 0x%x\n", dacRaw);

  // command byte: command type and power down selection
  // C2 C1 C0 X X PD1 PD0 X
  // C2 C1 C0: 0 1 0  (first nibble 0x4); write DAC register; requires writing 3 bytes
  // C2 C1 C0: 0 1 1; (first nibble 0x6); write DAC register and EEPROM; requires 5 bytes
  // PD1 PD0 : 0 0  ; normal mode
  // X       : ignored

  if (flagEeprom) cmdByte = 0x60;  // write to DAC and EEPROM
  else            cmdByte = 0x40;  // write to DAC only
  
  nTx      = 3;
  i2cAddr  = dacAddr;;

  
  // command register
  data[0]  = cmdByte;
  // DAC register
  dacRaw   = (dacRaw << 4) & 0xfff0;           // shift to leftmost position
  data[1]  = (uint8_t)((dacRaw >> 8) & 0xff);  // highest 8 bits
  data[2]  = (uint8_t)( dacRaw       & 0xf0);  // lowest  4 bits

  // printf("DAC raw value is %d\n", dacRaw);
  
  ftStatus = I2C_DeviceWrite(cHandle, i2cAddr, nTx, data, &nTxd, transOpt);
  if ((nTxd != nTx) && flagDebug) printf("wrong number of bytes; expected %d, transferred %d\n", nTx, nTxd);

  return ftStatus;
} //  ftdimcp_setLevel


// gets level of comparator
FT_STATUS ftdimcp_getLevel(FT_HANDLE cHandle, uint32_t dacAddr)
{
  FT_STATUS ftStatus;
  uint32_t  nRx;                   // number of bytes to write
  uint32_t  nRxd;                  // number of bytes that have been written
  uint8_t   data[10];              // data used for transfer
  uint32_t  transOpt;              // options used for transfer
  uint32_t  i2cAddr;               // i2c address of DAC


  // experimental, this does not work
  
  // data transfer options
  transOpt = 0x3;
  nRx      = 5;
  i2cAddr  = dacAddr;

  printf("try reading from DAC\n");
  data[0] = data[1] = data[2] = data[3] = data[4] = 0x0;
  ftStatus = I2C_DeviceRead(cHandle, i2cAddr, nRx, data, &nRxd, transOpt);
  printf("bytes read from DAC       %d\n", nRxd);
  printf("DAC setting is          0x%x\n", data[0]);
  printf("value0 read from DAC is 0x%x\n", data[1]);
  printf("value1 read from DAC is 0x%x\n", data[2]);
  printf("EEPROM Data byte 0 is   0x%x\n", data[3]);
  printf("EEPROM Data byte 1 is   0x%x\n", data[4]);

  return ftStatus;
} //  ftdimcp_read


// (un)sets LED
FT_STATUS ftdimpc_setLed(FT_HANDLE cHandle, uint32_t on)
{
  FT_STATUS ftStatus;
  uint8_t   direction;               // bitwise pin c0..c7, 1: out, 0: in
  uint8_t   value;                   // bitwise pin c0..c7, value
  uint8_t   pinled;                  // pin to which the LED is connected
   
  direction = FTDIMCP_GPIODIR;       // GPIO pin direction
  pinled    = FTDIMCP_PINLED;        // pin to set
  value     = (on & 0x1) << pinled;  // (un)set pin

  ftStatus  = FT_WriteGPIO(cHandle, direction, value);

  return ftStatus;
} // ftdimpc_setLed


// gets actual value of comparator output
FT_STATUS ftdimpc_getCompOutAct(FT_HANDLE cHandle, uint32_t *on)
{
  FT_STATUS ftStatus;

  uint8_t   value;                   // value of channel GPIO
  uint8_t   pin ;                    // pin to read

  pin       = FTDIMCP_PINACT;
  
  ftStatus  = FT_ReadGPIO(cHandle, &value);
  *on       = (value >> pin) & 0x1;

  return ftStatus; 
} // ftdimpc_getCompOutAct


// gets 'stretched' value of comparator output
FT_STATUS ftdimpc_getCompOutStretched(FT_HANDLE cHandle, uint32_t *on)
{
  FT_STATUS ftStatus;


  uint8_t   value;                   // value of channel GPIO
  uint8_t   pin ;                    // pin to read

  pin       = FTDIMCP_PINSTRETCH;
  
  ftStatus  = FT_ReadGPIO(cHandle, &value);
  *on       = (value >> pin) & 0x1;
  
  return ftStatus;
} // ftdimpc_getCompOutStretched
