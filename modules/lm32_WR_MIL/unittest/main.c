#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "wr_mil_piggy.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"

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
} MilPiggyFakeDevice;
MilPiggyFakeDevice my_fake_mil_piggy_device;

int main()
{
	MilPiggy_t mil_piggy;
	MilPiggy_init(&mil_piggy, (uint32_t*)&my_fake_mil_piggy_device);
	MilPiggy_writeCmd(&mil_piggy, 42);

	assert(my_fake_mil_piggy_device.MIL_WR_CMD == 42);


	printf("all tests successful\n");

	return 0;
}