#include "scu_mil.h"

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
    if (base[MIL_WR_RD_STATUS] & MIL_TRM_READY)
      break;
  }
  if (i > 0)
    return OKAY;
  else
    return TRM_NOT_FREE;
}

int write_mil(volatile unsigned int *base, short data, short fc_ifc_addr) {
  if (trm_free(base) == OKAY) {
    base[MIL_RD_WR_DATA] = data;
  } else {
    return TRM_NOT_FREE;
  }
  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = fc_ifc_addr;
    return OKAY;
  } else {
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

int read_mil(volatile unsigned int *base, short *data, short fc_ifc_addr) {
  int rcv_flags = 0;

  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = fc_ifc_addr;
  } else {
    return TRM_NOT_FREE;
  }
  rcv_flags = rcv_flag(base);
  if (rcv_flags == OKAY) {
    *data = base[MIL_RD_WR_DATA];
    return OKAY;
  } else if (rcv_flags == RCV_ERROR) {
      return RCV_ERROR;
  } else if (rcv_flags == RCV_TIMEOUT) {
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

int clear_filter_evt_mil(volatile unsigned int *base)
{
  unsigned int filterSize;         /* size of filter RAM     */
  uint32_t     *pFilterRAM;        /* RAM for event filters  */
  int          i;

  filterSize = (EV_FILT_LAST - EV_FILT_FIRST) + 1;
  //filterSize = (filterSize * 6) >> 2;          /* filtersize in units of 6 bits   */
  mprintf("filtersize: %d, base 0x%08x\n", filterSize, base);
  pFilterRAM = (uint32_t *)(base + (EV_FILT_FIRST));      /* address to filter RAM */
  for (i=0; i < filterSize; i++) pFilterRAM[i] = 0x0;
  mprintf("&pFilterRAM[0]: 0x%08x, &pFilterRAM[filterSize-1]: 0x%08x\n", &(pFilterRAM[0]), &(pFilterRAM[filterSize-1]));

   return OKAY;
} /* clear_filter_evt_MIL */

int set_filter_evt_mil(volatile unsigned int *base, unsigned short evt_code, unsigned short acc_number, unsigned short filter)
{
  //typedef struct uint6_t {
  //  uint32_t value : 6; 
  //} uint6_t; /* define 6 bit data type */
  
  uint32_t *pFilterRAM;             /* RAM for event filters      */
  //uint8_t help;                    /* helper variable            */
  int i;

  if (evt_code > 255)  return MIL_OUT_OF_RANGE;
  if (acc_number > 15) return MIL_OUT_OF_RANGE;
  
  pFilterRAM = (uint32_t *)(base + (EV_FILT_FIRST));      /* address to filter RAM */
  pFilterRAM[acc_number*256+evt_code] = filter;
  //for (i=0;i<3842;i++) pFilterRAM[i]=0x1;
  //
  
  mprintf("pFilter: 0x%08x, &pFilter[evt_code*16+acc_number]: 0x%08x\n", pFilterRAM, &(pFilterRAM[evt_code*16+acc_number]));

  return OKAY;
} /* set_filter_evt_MIL */

int enable_filter_evt_mil(volatile unsigned int *base)
{
  unsigned int statusReg;

  read_statusreg_evt_mil(base, &statusReg);
  //mprintf("enable filter - statusReg: 0x%08x\n", statusReg);
  statusReg = statusReg | MIL_EV_FILTER_ON;
  //mprintf("enable filter - statusReg: 0x%08x\n", statusReg);
  write_statusreg_evt_mil(base, statusReg);
  
  return OKAY;
} /* eneable_filer_evt_mil */

int disable_filter_evt_mil(volatile unsigned int *base)
{
  unsigned int statusReg;

  read_statusreg_evt_mil(base, &statusReg);
  //mprintf("disable filter - statusReg: 0x%08x\n", statusReg);
  //mprintf("MIL_EVT_FILTER_ON: 0x%08x, !MIL_EVT_FILTER_IN: 0x%08x\n", MIL_EV_FILTER_ON, ~MIL_EV_FILTER_ON);
  statusReg = statusReg & (~MIL_EV_FILTER_ON);
  //mprintf("disable filter - statusReg: 0x%08x\n", statusReg);
  write_statusreg_evt_mil(base, statusReg);
    
  return OKAY;
} /* disable_fitler_evt_mil */

int write_statusreg_evt_mil(volatile unsigned int *base, unsigned int data)
{
  uint32_t *pControlRegister;  /* control register of event filter */

  pControlRegister  = (uint32_t *)(base + MIL_WR_RD_STATUS);
  *pControlRegister = data;

  return OKAY;
} /* write_statusreg_evt_mil */

int read_statusreg_evt_mil(volatile unsigned int *base, unsigned int *data)
{
  uint32_t *pControlRegister;  /* control register of event filter */

  pControlRegister  = (uint32_t *)(base + MIL_WR_RD_STATUS);
  *data = *pControlRegister;

  return OKAY;
} /* read statusreg_evt_mil */

int enable_lemo_pulse_evt_mil(volatile unsigned int *base, unsigned int lemo)
{
  unsigned int statusReg;
  
  uint32_t *pConfigRegister;
  uint32_t configReg;

  if (lemo > 4) return MIL_OUT_OF_RANGE;

  /* disable gate mode */
  read_statusreg_evt_mil(base, &statusReg);
  if (lemo == 1) statusReg = statusReg & ~MIL_PULS1_FRAME;
  if (lemo == 2) statusReg = statusReg & ~MIL_PULS2_FRAME;
  write_statusreg_evt_mil(base, statusReg);

  /* enable output */
  pConfigRegister = (uint32_t *)(base + MIL_WR_RF_LEMO_CONF);
  configReg = *pConfigRegister;
  if (lemo == 1) configReg = configReg | LEMO_OUT_EN1 | LEMO_EVENT_EN1;
  if (lemo == 2) configReg = configReg | LEMO_OUT_EN2 | LEMO_EVENT_EN2;
  if (lemo == 3) configReg = configReg | LEMO_OUT_EN3 | LEMO_EVENT_EN3;
  if (lemo == 4) configReg = configReg | LEMO_OUT_EN4 | LEMO_EVENT_EN4;
  *pConfigRegister = configReg;

  return OKAY;
} /* enable_lemo_pulse_evt_mil */

int enable_lemo_gate_evt_mil(volatile unsigned int *base, unsigned int lemo)
{
  unsigned int statusReg;
  
  uint32_t *pConfigRegister;
  uint32_t configReg;

  if (lemo > 2) return MIL_OUT_OF_RANGE;
  
  /* enable gate mode */
  read_statusreg_evt_mil(base, &statusReg);
  if (lemo == 1) statusReg = statusReg | MIL_PULS1_FRAME;
  if (lemo == 2) statusReg = statusReg | MIL_PULS2_FRAME;
  write_statusreg_evt_mil(base, statusReg);

  /* enable output */
  pConfigRegister = (uint32_t *)(base + MIL_WR_RF_LEMO_CONF);
  configReg = *pConfigRegister;
  if (lemo == 1) configReg = configReg | LEMO_EVENT_EN1;
  if (lemo == 2) configReg = configReg | LEMO_EVENT_EN2;
  *pConfigRegister = configReg;
  
  return OKAY;  
} /*enable_lemo_gate_evt_mil */

int disable_lemo_evt_mil(volatile unsigned int *base, unsigned int lemo)
{
  unsigned int statusReg;
  
  uint32_t *pConfigRegister;
  uint32_t configReg;

  if (lemo > 4) return MIL_OUT_OF_RANGE;

  /* disable gate mode */
  read_statusreg_evt_mil(base, &statusReg);
  if (lemo == 1) statusReg = statusReg & ~MIL_PULS1_FRAME;
  if (lemo == 2) statusReg = statusReg & ~MIL_PULS2_FRAME;
  write_statusreg_evt_mil(base, statusReg);

  /* disable output */
  pConfigRegister = (uint32_t *)(base + MIL_WR_RF_LEMO_CONF);
  configReg = *pConfigRegister;
  if (lemo == 1) configReg = configReg & ~LEMO_OUT_EN1 & ~LEMO_EVENT_EN1;
  if (lemo == 2) configReg = configReg & ~LEMO_OUT_EN2 & ~LEMO_EVENT_EN2;
  if (lemo == 3) configReg = configReg & ~LEMO_OUT_EN3 & ~LEMO_EVENT_EN3;
  if (lemo == 4) configReg = configReg & ~LEMO_OUT_EN4 & ~LEMO_EVENT_EN4;
  *pConfigRegister = configReg;

  return OKAY;
  
} /* disable_lemo_evt_mil */


int fifo_notempty_evt_mil(volatile unsigned int *base)
{
  unsigned int regValue;
  int          fifoNotEmpty;

  read_statusreg_evt_mil(base, &regValue);
  fifoNotEmpty = regValue & MIL_EV_FIFO_NE;
  
  return (fifoNotEmpty);
} /* fifo_isempty_evt_mil */

int clear_fifo_evt_mil(volatile unsigned int *base)
{
  *(base + RD_CLR_EV_FIFO) = 0x1; /* check: value */

  return OKAY;
} /* clear_fifo_evt_mil */

int pop_fifo_evt_mil(volatile unsigned int *base, unsigned int *data)
{
  unsigned int *pFIFO;

  pFIFO = (unsigned int *)(base + RD_CLR_EV_FIFO);

  *data = *pFIFO;
  
  return OKAY;
} /* pop_fifo_evt_mil */
