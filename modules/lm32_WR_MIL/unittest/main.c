#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>

//#include "wr_mil_piggy.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"
#include "wr_mil_utils.h"

typedef struct
{
	uint32_t MIL_RD_WR_DATA;
	uint32_t MIL_WR_CMD;
	uint32_t MIL_WR_RD_STATUS;
	uint32_t RD_CLR_NO_VW_CNT;
	uint32_t RD_WR_NOT_EQ_CNT;
	uint32_t RD_CLR_EV_FIFO;
	uint32_t RD_CLR_TIMER;
	uint32_t RD_WR_DLY_TIMER;
	uint32_t RD_CLR_WAIT_TIMER;
	uint32_t WR_RF_LEMO_CONF;
	uint32_t WR_RD_LEMO_DAT;
	uint32_t RD_LEMO_INP_A;
	uint32_t EV_FILT_FIRST;
	uint32_t EV_FILT_LAST;
} MilPiggyFakeDevice;
MilPiggyFakeDevice my_fake_mil_piggy_device;

typedef struct 
{
	uint32_t ECA_QUEUE_QUEUE_ID_GET;
	uint32_t ECA_QUEUE_POP_OWR;
	uint32_t ECA_QUEUE_FLAGS_GET      ;
	uint32_t ECA_QUEUE_NUM_GET;
	uint32_t ECA_QUEUE_EVENT_ID_HI_GET;
	uint32_t ECA_QUEUE_EVENT_ID_LO_GET;
	uint32_t ECA_QUEUE_PARAM_HI_GET;
	uint32_t ECA_QUEUE_PARAM_LO_GET;
	uint32_t ECA_QUEUE_TAG_GET;
	uint32_t ECA_QUEUE_TEF_GET;
	uint32_t ECA_QUEUE_DEADLINE_HI_GET;
	uint32_t ECA_QUEUE_DEADLINE_LO_GET;
	uint32_t ECA_QUEUE_EXECUTED_HI_GET;
	uint32_t ECA_QUEUE_EXECUTED_LO_GET;
} ECAQueueFakeDevice;
ECAQueueFakeDevice my_fake_eca_queue_device;

int main() 
{
	////////////////////////////////////////
	// MilPiggy module tests
	////////////////////////////////////////
	// volatile MilPiggyRegs *mil_piggy = (volatile MilPiggyRegs *)&my_fake_mil_piggy_device;

	// uint32_t mil_piggy_test_value = 42;
	// MilPiggy_writeCmd(mil_piggy, mil_piggy_test_value);
	// assert(my_fake_mil_piggy_device.MIL_WR_CMD == mil_piggy_test_value);

	// // test lemo connector settings
	// MilPiggy_lemoOut1Enable(mil_piggy);
	// assert(my_fake_mil_piggy_device.WR_RF_LEMO_CONF & MIL_LEMO_OUT_EN1);
	// MilPiggy_lemoOut2Enable(mil_piggy);
	// assert(my_fake_mil_piggy_device.WR_RF_LEMO_CONF & MIL_LEMO_OUT_EN2);
	// // test lemo connector on / off
	// MilPiggy_lemoOut2High(mil_piggy);
	// assert(my_fake_mil_piggy_device.WR_RD_LEMO_DAT & MIL_LEMO_DAT2);
	// MilPiggy_lemoOut2Low(mil_piggy);
	// assert(!(my_fake_mil_piggy_device.WR_RD_LEMO_DAT & MIL_LEMO_DAT2));


	// ////////////////////////////////////////
	// // ECA queue module tests
	// ////////////////////////////////////////
	// ECAQueueRegs *eca_queue = (ECAQueueRegs *)&my_fake_eca_queue_device;

	// // test getEvtId function
	// uint32_t eca_queue_test_id_hi = 0x12345678u;
	// uint32_t eca_queue_test_id_lo = 0x01234567u;
	// my_fake_eca_queue_device.ECA_QUEUE_EVENT_ID_HI_GET = eca_queue_test_id_hi;
	// my_fake_eca_queue_device.ECA_QUEUE_EVENT_ID_LO_GET = eca_queue_test_id_lo;
	// EvtId_t evt_id;
	// ECAQueue_getEvtId(eca_queue, &evt_id);
	// assert(evt_id.part.hi == eca_queue_test_id_hi);
	// assert(evt_id.part.lo == eca_queue_test_id_lo);


	// test timestamp conversion
	uint32_t sec_old, ms_old;
	for (uint64_t i = 0; i < 20000; ++i)
	{
		uint64_t TAI = 0x14c13f9783795370 + i*1000000;
		uint32_t EVT_UTC[5];
		make_mil_timestamp(TAI, EVT_UTC);
		for (int i = 0; i < 5; ++i) EVT_UTC[i] >>= 8;
		//printf("TAI_ms %" PRIu64 "\n", TAI_ms);
		uint32_t ms   = (EVT_UTC[0] << 2) | ((EVT_UTC[1] >> 6));
		uint32_t sec  = (EVT_UTC[1] & 0x0000002f) << 24;
		         sec |= (EVT_UTC[2] & 0x000000ff) << 16;
		         sec |= (EVT_UTC[3] & 0x000000ff) << 8;
		         sec |= (EVT_UTC[4] & 0x000000ff) << 0;
		//printf("sec %d  :  ms %d   \n",sec, ms);

		// assert some critical properties of the second and milisecond values
		if (i != 0)	{
			assert((ms_old+1)%1000 == ms);
			if (ms==0) {
				assert(sec_old+1 == sec);
			}
		}

		sec_old = sec;
		ms_old  = ms;
	}

	for (uint64_t i = 0; i < 2000; ++i)
	{
		uint64_t TAI = 0x14c14a295ab81178 + i*1000000;
		uint32_t EVT_UTC[5];
		make_mil_timestamp(TAI, EVT_UTC);
		uint64_t my_sec   = TAI/UINT64_C(1000000)-UINT64_C(1199142000000);
		uint32_t my_sec32 = my_sec / 1000;

		for (int i = 0; i < 5; ++i) EVT_UTC[i] >>= 8;
		uint32_t ms   = (EVT_UTC[0] << 2) | ((EVT_UTC[1] >> 6));
		uint32_t sec  = 0;
		         sec |= (EVT_UTC[1] & 0x0000003f) << 24;
		         sec |= (EVT_UTC[2] & 0x000000ff) << 16;
		         sec |= (EVT_UTC[3] & 0x000000ff) << 8;
		         sec |= (EVT_UTC[4] & 0x000000ff) << 0;
		//printf("seconds since 01/01/2008 %d.%d   \n",sec, ms);

		// assert some critical properties of the second and milisecond values
		if (i != 0)	{
			assert((ms_old+1)%1000 == ms);
			if (ms==0) {
				assert(sec_old+1 == sec);
			}
		}

		sec_old = sec;
		ms_old  = ms;
	}


	printf("all tests successful\n");

	return 0;
}