#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "wr_mil_piggy.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"


/***********************************************************
 * 
 * defintion of LEMO config register
 * 
 * bits 0..7: see below
 * bits 8..31: unused
 *
 ***********************************************************/
#define   MIL_LEMO_OUT_EN1    0x0001    // '1' ==> LEMO 1 configured as output (MIL Piggy)
#define   MIL_LEMO_OUT_EN2    0x0002    // '1' ==> LEMO 2 configured as output (MIL Piggy)
#define   MIL_LEMO_OUT_EN3    0x0004    // '1' ==> LEMO 3 configured as output (SIO)
#define   MIL_LEMO_OUT_EN4    0x0008    // '1' ==> LEMO 4 configured as output (SIO)
#define   MIL_LEMO_EVENT_EN1  0x0010    // '1' ==> LEMO 1 can be controlled by event (MIL Piggy)
#define   MIL_LEMO_EVENT_EN2  0x0020    // '1' ==> LEMO 2 can be controlled by event (MIL Piggy)
#define   MIL_LEMO_EVENT_EN3  0x0040    // '1' ==> LEMO 3 can be controlled by event (unused?)
#define   MIL_LEMO_EVENT_EN4  0x0080    // '1' ==> LEMO 4 can be controlled by event (unused?) 


/***********************************************************
 * 
 * defintion of LEMO data register
 * in case LEMO outputs are not controlled via events,
 * this register can be used to control them
 * 
 * bits 0..3: see below
 * bits 4..31: unused
 *
 ***********************************************************/
#define   MIL_LEMO_DAT1    0x0001    // '1' ==> LEMO 1 is switched active HIGH (MIL Piggy & SIO)
#define   MIL_LEMO_DAT2    0x0002    // '1' ==> LEMO 2 is switched active HIGH (MIL Piggy & SIO)
#define   MIL_LEMO_DAT3    0x0004    // '1' ==> LEMO 3 is switched active HIGH (SIO)
#define   MIL_LEMO_DAT4    0x0008    // '1' ==> LEMO 4 is switched active HIGH (SIO)



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
	volatile MilPiggyRegs *mil_piggy = MilPiggy_init((uint32_t*)&my_fake_mil_piggy_device);

	uint32_t mil_piggy_test_value = 42;
	MilPiggy_writeCmd(mil_piggy, mil_piggy_test_value);
	assert(my_fake_mil_piggy_device.MIL_WR_CMD == mil_piggy_test_value);

	// test lemo connector settings
	MilPiggy_lemoOut1Enable(mil_piggy);
	assert(my_fake_mil_piggy_device.WR_RF_LEMO_CONF & MIL_LEMO_OUT_EN1);
	MilPiggy_lemoOut2Enable(mil_piggy);
	assert(my_fake_mil_piggy_device.WR_RF_LEMO_CONF & MIL_LEMO_OUT_EN2);
	// test lemo connector on / off
	MilPiggy_lemoOut2High(mil_piggy);
	assert(my_fake_mil_piggy_device.WR_RD_LEMO_DAT & MIL_LEMO_DAT2);
	MilPiggy_lemoOut2Low(mil_piggy);
	assert(!(my_fake_mil_piggy_device.WR_RD_LEMO_DAT & MIL_LEMO_DAT2));


	////////////////////////////////////////
	// ECA queue module tests
	////////////////////////////////////////
	ECAQueue_t *eca_queue = ECAQueue_init((uint32_t*)&my_fake_eca_queue_device);

	// test getEvtId function
	uint32_t eca_queue_test_id_hi = 0x12345678u;
	uint32_t eca_queue_test_id_lo = 0x01234567u;
	my_fake_eca_queue_device.ECA_QUEUE_EVENT_ID_HI_GET = eca_queue_test_id_hi;
	my_fake_eca_queue_device.ECA_QUEUE_EVENT_ID_LO_GET = eca_queue_test_id_lo;
	EvtId_t evt_id;
	ECAQueue_getEvtId(eca_queue, &evt_id);
	assert(evt_id.part.hi == eca_queue_test_id_hi);
	assert(evt_id.part.lo == eca_queue_test_id_lo);




	printf("all tests successful\n");

	return 0;
}