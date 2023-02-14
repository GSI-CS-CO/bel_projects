/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2011 CERN (www.cern.ch)
 * Author: Grzegorz Daniluk <grzegorz.daniluk@cern.ch>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */


#include "board.h"
#include "hw/wrc_syscon_regs.h"
#include "hw/rawmem.h"

void timer_init(int enable)
{
	writel( SYSC_TCR_ENABLE, (void*) BASE_SYSCON + SYSC_REG_TCR );
}

uint32_t timer_get_tics()
{
	return readl( (void*) BASE_SYSCON + SYSC_REG_TVR );
}
