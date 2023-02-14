/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <wrc.h>
#include <syscon.h>
//#include <shell.h>

static int usleep_lpj; /* loops per jiffy */


static inline void __delay(int count)
{
	while (count-- > 0)
		asm("");
}


static int verify_lpj(int lpj)
{
	unsigned long j;

	/* wait for the beginning of a tick */
	j = timer_get_tics() + 1;
	while (timer_get_tics() < j)
		;

	__delay(lpj);

	/* did it expire? */
	j = timer_get_tics() - j;
	if (0)
		pp_printf("check %i: %li\n", lpj, j);
	return j;
}

void usleep_init(void)
{
	int lpj = 1024, test_lpj;
	int step = 1024;

	/* Increase until we get over it */
	while (verify_lpj(lpj) == 0) {
		lpj += step;
		step *= 2;
	}
	/* Ok, now we are over; half again and restart */
	lpj /= 2; step /= 4;

	/* So, *this* jpj is lower, and with two steps we are higher */
	while (step) {
		test_lpj = lpj + step;
		if (verify_lpj(test_lpj) == 0)
			lpj = test_lpj;
		step /= 2;
	}
	usleep_lpj = lpj;
	//main_dbg("calibrating usleep(): loops per jiffy = %i\n", lpj);
}

/* lpj is around 20800 on the spec: the above calculation overflows at 200ms */
int usleep(unsigned usec)
{
	/* Sleep 10ms each time, so we support 20x faster cards */
	const int step = 10 * 1000;
	const int usec_per_jiffy = 1000 * 1000 / TICS_PER_SECOND;
	const int count_per_step = usleep_lpj * step / usec_per_jiffy;

	while (usec > step)  {
		__delay(count_per_step);
		usec -= step;
	}
	__delay(usec * usleep_lpj / usec_per_jiffy);
	return 0;
}
