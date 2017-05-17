#ifndef WR_MIL_PIGGY_H_
#define WR_MIL_PIGGY_H_

#include <stdint.h>

typedef struct 
{
	uint32_t rd_wr_data;        // MIL_RD_WR_DATA;
	uint32_t wr_cmd;            // MIL_WR_CMD;
	uint32_t wr_rd_status;      // MIL_WR_RD_STATUS;
	uint32_t rd_clr_no_vw_cnt;  // RD_CLR_NO_VW_CNT;
	uint32_t rd_wr_not_eq_cnt;  // RD_WR_NOT_EQ_CNT;
	uint32_t rd_clr_ev_fifo;    // RD_CLR_EV_FIFO;
	uint32_t rd_clr_timer;      // RD_CLR_TIMER;
	uint32_t rd_wr_dly_timer;   // RD_WR_DLY_TIMER;
	uint32_t rd_clr_wait_timer; // RD_CLR_WAIT_TIMER;
	uint32_t wr_rf_lemo_conf;   // WR_RF_LEMO_CONF;
	uint32_t wr_rd_lemo_dat;    // WR_RD_LEMO_DAT;
	uint32_t rd_lemo_inp_a;     // RD_LEMO_INP_A;
	uint32_t ev_filt_first;     // EV_FILT_FIRST;
	uint32_t ev_filt_last;      // EV_FILT_LAST;	
} MilPiggyRegs;

volatile MilPiggyRegs *MilPiggy_init(uint32_t *device_addr);

/* write the lower 16 bit of cmd to Mil device bus */
void MilPiggy_writeCmd(volatile MilPiggyRegs *piggy, uint32_t cmd);

void MilPiggy_lemoOut1Enable(volatile MilPiggyRegs *piggy);
void MilPiggy_lemoOut2Enable(volatile MilPiggyRegs *piggy);
void MilPiggy_lemoOut1High(volatile MilPiggyRegs *piggy);
void MilPiggy_lemoOut2High(volatile MilPiggyRegs *piggy);
void MilPiggy_lemoOut1Low(volatile MilPiggyRegs *piggy);
void MilPiggy_lemoOut2Low(volatile MilPiggyRegs *piggy);

#endif 
