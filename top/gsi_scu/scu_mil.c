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
