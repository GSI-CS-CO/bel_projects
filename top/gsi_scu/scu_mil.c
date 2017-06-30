#include "scu_mil.h"
#include "aux.h"

/***********************************************************
 ***********************************************************
 *  
 * 1st part: original MIL bus library
 *
 ***********************************************************
 ***********************************************************/

void clear_receive_flag(volatile unsigned int *base) {
  unsigned short rcv_data = 0;
  usleep(50); // wait 50us, because an initiated mil_read takes 20us for trm function code and ifc address
              // and the possible receive pattern needs also 20us.
  if (rcv_flag(base)) {
    rcv_data = base[MIL_RD_WR_DATA];  // rcv flag is active, so clear flag with reading mil receive register
  }
}

int trm_free(volatile unsigned int *base) {
  int i = MAX_TST_CNT;
  
  for (i = MAX_TST_CNT; i > 0; i--) {
    if (base[MIL_WR_RD_STATUS] & MIL_TRM_READY) {
      break;
    }
  }
  if (i > 0)
    return OKAY;
  else
    return TRM_NOT_FREE;
}

int write_mil(volatile unsigned int *base, short data, short fc_ifc_addr) {
  atomic_on();
  if (trm_free(base) == OKAY) {
    base[MIL_RD_WR_DATA] = data;
  } else {
    atomic_off();
    return TRM_NOT_FREE;
  }
  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = fc_ifc_addr;
    atomic_off();
    return OKAY;
  } else {
    atomic_off();
    return TRM_NOT_FREE;
  }
}

int rcv_flag(volatile unsigned int *base) {
  unsigned short status = 0;
  int i = MAX_TST_CNT;
  
  for (i = MAX_TST_CNT; i > 0; i--) {
    status = (base[MIL_WR_RD_STATUS] & (MIL_RCV_READY | MIL_RCV_ERROR));
    if (status) {
      break;
    }
  }
  if (i > 0) {
    if ((status & MIL_RCV_READY) > 0) {
      return OKAY;   // received data
    } else {
      base[MIL_WR_RD_STATUS] = base[MIL_WR_RD_STATUS]; // clear rcv error bit
      return RCV_ERROR;  // rcv error is set
    }
  } else {
    return RCV_TIMEOUT;  // rcv timeout
  }  
}
int status_mil(volatile unsigned int *base, unsigned short *status) {
  atomic_on();
  *status = base[MIL_WR_RD_STATUS];
  atomic_off();
  return OKAY;
}


int read_mil(volatile unsigned int *base, short *data, short fc_ifc_addr) {
  int rcv_flags = 0;

  atomic_on();
  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = fc_ifc_addr;
  } else {
    atomic_off();
    return TRM_NOT_FREE;
  }
  rcv_flags = rcv_flag(base);
  if (rcv_flags == OKAY) {
    *data = base[MIL_RD_WR_DATA];
    atomic_off();
    return OKAY;
  } else if (rcv_flags == RCV_ERROR) {
    atomic_off();
    return RCV_ERROR;
  } else if (rcv_flags == RCV_TIMEOUT) {
    atomic_off();
    return RCV_TIMEOUT;
  }
}

void run_mil_test(volatile unsigned int *base, unsigned char ifc_addr) {
  int   test_loop_64k = 0;
  int   rcv_timeout_cnt = 0;
  int   rcv_error_cnt = 0;
  int   send_error_cnt = 0;
  int   data_error_cnt = 0;
  int   read_mil_status = 0;
  unsigned short rcv_data = 0;
  unsigned short test_pattern = 0;
  
  unsigned short wr_echo_ifc = 0x13 << 8; // place function code wr echo reg (0x13) to high byte; low byte holds ifc-card-address
  unsigned short rd_echo_ifc = 0x89 << 8; // place function code rd echo reg (0x89) to high byte; low byte holds ifc-card-address  
  
  wr_echo_ifc |= ifc_addr;
  rd_echo_ifc |= ifc_addr;
    

  mprintf("Mil_Base: 0x%x\n", base);
  clear_receive_flag(base);
  while(1) {
   
    if (write_mil(base, test_pattern, wr_echo_ifc) == OKAY) {
      read_mil_status = read_mil(base, &rcv_data, rd_echo_ifc);
      if (read_mil_status == OKAY) {
	      if (test_pattern == rcv_data) {
          if (test_pattern == 0xffff) {
            test_loop_64k++;
            mprintf("loop_64k: %d  data_err: %d  rcv_to: %d  rcv_err: %d\n",
                     test_loop_64k, data_error_cnt, rcv_timeout_cnt, rcv_error_cnt);
          }
        } else { // test_pattern not equal with rcv_data
          mprintf("pattern not equal: test_pattern: 0x%x rcv_data 0x%x\n", test_pattern, rcv_data);
          data_error_cnt++;
        }
      } else if (read_mil_status == TRM_NOT_FREE) {
          send_error_cnt++;
          mprintf("mil_rd send error: 0x%x\n", send_error_cnt);
      } else if (read_mil_status == RCV_ERROR) {
          rcv_error_cnt++;
          mprintf("mil_rd rcv error: 0x%x\n", rcv_error_cnt);
      } else if (read_mil_status == RCV_TIMEOUT) {
          rcv_timeout_cnt++;
          mprintf("mil_rcv timeout: 0x%x\n", rcv_timeout_cnt);
      } else {
        mprintf("unknown error");
      }
    }
    else { // send error
      send_error_cnt++;
      mprintf("mil_wr send error: 0x%x\n", send_error_cnt);
    }
    test_pattern++;
  }
}

/***********************************************************
 ***********************************************************
 * 
 * 2st part:  (new) MIL bus library
 *
 ***********************************************************
 ***********************************************************/
int16_t writeDevMil(volatile uint32_t *base, uint16_t  ifbAddr, uint16_t  fctCode, uint16_t  data)
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated
  
  uint16_t fc_ifb_addr;

  fc_ifb_addr = ifbAddr | (fctCode << 8);

  return (int16_t)write_mil((unsigned int *)base, (short)data, (short)fc_ifb_addr);
} // writeDevMil

int16_t readDevMil(volatile uint32_t *base, uint16_t  ifbAddr, uint16_t  fctCode, uint16_t  *data)
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated

  uint16_t fc_ifb_addr;

  fc_ifb_addr = ifbAddr | (fctCode << 8);

  return (int16_t)read_mil((unsigned int *)base, (short *)data, (short)fc_ifb_addr);
} //writeDevMil

int16_t echoTestDevMil(volatile uint32_t *base, uint16_t  ifbAddr, uint16_t data)
{
  int32_t  busStatus;
  uint16_t rData = 0x0;

  busStatus = writeDevMil(base, ifbAddr, FC_WR_IFC_ECHO, data);
  if (busStatus != MIL_STAT_OK) return busStatus;

  busStatus = readDevMil(base, ifbAddr, FC_RD_IFC_ECHO, &rData);
  if (busStatus != MIL_STAT_OK) return busStatus;

  if (data != rData) return MIL_STAT_ERROR;
  else               return MIL_STAT_OK;
} //echoTestDevMil


int16_t clearFilterEvtMil(volatile uint32_t *base)
{
  uint32_t filterSize;         // size of filter RAM     
  uint32_t *pFilterRAM;        // RAM for event filters
  uint32_t i;

  filterSize = (MIL_REG_EV_FILT_LAST >> 2) - (MIL_REG_EV_FILT_FIRST >> 2) + 1;
  // mprintf("filtersize: %d, base 0x%08x\n", filterSize, base);

  pFilterRAM = (uint32_t *)(base + (MIL_REG_EV_FILT_FIRST >> 2));      // address to filter RAM 
  for (i=0; i < filterSize; i++) pFilterRAM[i] = 0x0;

  // mprintf("&pFilterRAM[0]: 0x%08x, &pFilterRAM[filterSize-1]: 0x%08x\n", &(pFilterRAM[0]), &(pFilterRAM[filterSize-1]));

  return MIL_STAT_OK;
} //clearFiterEvtMil

int16_t setFilterEvtMil(volatile uint32_t *base, uint16_t evtCode, uint16_t virtAcc, uint32_t filter)
{
  uint32_t *pFilterRAM;        // RAM for event filters

  if (virtAcc > 15) return MIL_STAT_OUT_OF_RANGE;

  pFilterRAM = (uint32_t*)(base + (uint32_t)(MIL_REG_EV_FILT_FIRST >> 2));  // address to filter RAM 

  pFilterRAM[virtAcc*256+evtCode] = filter;

  // mprintf("pFilter: 0x%08x, &pFilter[evt_code*16+acc_number]: 0x%08x\n", pFilterRAM, &(pFilterRAM[evtCode*16+virtAcc]));

  return MIL_STAT_OK;
} //setFilterEvtMil

int16_t enableFilterEvtMil(volatile uint32_t *base)
{
  uint32_t regValue;

  readCtrlStatRegEvtMil(base, &regValue);
  regValue = regValue | MIL_CTRL_STAT_EV_FILTER_ON;
  writeCtrlStatRegEvtMil(base, regValue);
  
  return MIL_STAT_OK;
} //enableFilterEvtMil


int16_t disableFilterEvtMil(volatile uint32_t *base)
{
  uint32_t regValue;

  readCtrlStatRegEvtMil(base, &regValue);
  regValue = regValue & (MIL_CTRL_STAT_EV_FILTER_ON);
  writeCtrlStatRegEvtMil(base, regValue);
    
  return MIL_STAT_OK;
} // disableFilterEvtMil

int16_t writeCtrlStatRegEvtMil(volatile uint32_t *base, uint32_t value)
{
  uint32_t *pControlRegister;  // control register of event filter

  pControlRegister  = (uint32_t *)(base + (MIL_REG_WR_RD_STATUS >> 2));
  *pControlRegister = value;

  return MIL_STAT_OK;
} // writeCtrlStatRegMil

int16_t readCtrlStatRegEvtMil(volatile uint32_t *base, uint32_t *value)
{
  uint32_t *pControlRegister;  // control register of event filter

  pControlRegister  = (uint32_t *)(base + (MIL_REG_WR_RD_STATUS >> 2));
  *value = *pControlRegister;

  return MIL_STAT_OK;
} //readCtrlStatRegMil

uint16_t fifoNotemptyEvtMil(volatile uint32_t *base)
{
  uint32_t regValue;
  uint16_t fifoNotEmpty;

  readCtrlStatRegEvtMil(base, &regValue);
  fifoNotEmpty = (uint16_t)(regValue & MIL_CTRL_STAT_EV_FIFO_NE);
  
  return (fifoNotEmpty);
} // fifoNotemptyEvtMil

int16_t clearFifoEvtMil(volatile uint32_t *base)
{
  uint32_t *pFIFO;

  pFIFO = (uint32_t *)(base + (MIL_REG_RD_CLR_EV_FIFO >> 2));
  *pFIFO = 0x1; // check value!!!

  return MIL_STAT_OK;
} // clearFifoEvtMil

int16_t popFifoEvtMil(volatile uint32_t *base, uint32_t *evtData)
{
  uint32_t *pFIFO;

  pFIFO = (uint32_t *)(base + (MIL_REG_RD_CLR_EV_FIFO >> 2));

  *evtData = *pFIFO;
  
  return MIL_STAT_OK;
} // popFifoEvtMil

int16_t configLemoPulseEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;
  
  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;

  // disable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // enable output
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue | MIL_LEMO_OUT_EN1 | MIL_LEMO_EVENT_EN1;
  if (lemo == 2) confRegValue = confRegValue | MIL_LEMO_OUT_EN2 | MIL_LEMO_EVENT_EN2;
  if (lemo == 3) confRegValue = confRegValue | MIL_LEMO_OUT_EN3 | MIL_LEMO_EVENT_EN3;
  if (lemo == 4) confRegValue = confRegValue | MIL_LEMO_OUT_EN4 | MIL_LEMO_EVENT_EN4;
  *pConfigRegister = confRegValue;

  return MIL_STAT_OK;
} // configLemoPulseEvtMil

int16_t configLemoGateEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;

  if (lemo > 2) return MIL_STAT_OUT_OF_RANGE;
  
  // enable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue | MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue | MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // enable output
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue | MIL_LEMO_EVENT_EN1;
  if (lemo == 2) confRegValue = confRegValue | MIL_LEMO_EVENT_EN2;
  *pConfigRegister = confRegValue;
  
  return MIL_STAT_OK;  
} //enableLemoGateEvtMil

int16_t configLemoOutputEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;
  
  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;

  // disable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // enable output for programable operation
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue | MIL_LEMO_OUT_EN1;
  if (lemo == 2) confRegValue = confRegValue | MIL_LEMO_OUT_EN2;
  if (lemo == 3) confRegValue = confRegValue | MIL_LEMO_OUT_EN3;
  if (lemo == 4) confRegValue = confRegValue | MIL_LEMO_OUT_EN4;
  *pConfigRegister = confRegValue;

  return MIL_STAT_OK; 
} //configLemoOutputEvtMil

int16_t setLemoOutputEvtMil(volatile uint32_t *base, uint32_t lemo, uint32_t on)
{
  uint32_t *pLemoDataRegister;

  uint32_t dataRegValue;

  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;
  if (on > 1)   return MIL_STAT_OUT_OF_RANGE;

  // read current value of register
  pLemoDataRegister = (uint32_t *)(base + (MIL_REG_WR_RD_LEMO_DAT >> 2));
  dataRegValue = *pLemoDataRegister;

  // modify value for register
  if (on) {
    if (lemo == 1) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN1;
    if (lemo == 2) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN2;
    if (lemo == 3) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN3;
    if (lemo == 4) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN4;
  } // if on
  else {
    if (lemo == 1) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN1;
    if (lemo == 2) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN2;
    if (lemo == 3) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN3;
    if (lemo == 4) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN4;
  } //else if on

  //write new value to register
  *pLemoDataRegister = dataRegValue;

  return MIL_STAT_OK;
} //setLemoOutputEvtMil


int16_t disableLemoEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;

  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;

  // disable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // disable output
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN1 & ~MIL_LEMO_EVENT_EN1;
  if (lemo == 2) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN2 & ~MIL_LEMO_EVENT_EN2;
  if (lemo == 3) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN3 & ~MIL_LEMO_EVENT_EN3;
  if (lemo == 4) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN4 & ~MIL_LEMO_EVENT_EN4;
  *pConfigRegister = confRegValue;

  return MIL_STAT_OK;
} // disableLemoEvtMil


