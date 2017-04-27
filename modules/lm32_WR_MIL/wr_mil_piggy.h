#ifndef WR_MIL_PIGGY_H_
#define WR_MIL_PIGGY_H_

#include <stdint.h>

typedef struct 
{
	volatile uint32_t *pMilPiggy;
	volatile uint32_t *pReadWriteData;
	volatile uint32_t *pWriteCmd;
} MilPiggy_t;

void MilPiggy_init(MilPiggy_t *piggy, uint32_t *device_addr);

/* write the lower 16 bit of cmd to Mil device bus */
void MilPiggy_writeCmd(MilPiggy_t *piggy, uint32_t cmd);

#endif 
