#ifndef __WRC_H__
#define __WRC_H__

/*
 * This header includes all generic prototypes that were missing
 * in earlier implementations. For example, the monitor is only
 * one function and doesn't deserve an header of its own.
 * Also, this brings in very common and needed headers
 */
#include <inttypes.h>
#include <syscon.h>
#include <pp-printf.h>
#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf

#undef offsetof
#define offsetof(TYPE, MEMBER) ((int) &((TYPE *)0)->MEMBER)
#undef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void wrc_mon_gui(void);
void shell_init(void);
int wrc_log_stats(uint8_t onetime);
void wrc_debug_printf(int subsys, const char *fmt, ...);


/* This is in the library, somewhere */
extern int abs(int val);

/* The following from ptp-noposix */
extern void wr_servo_reset(void);
void update_rx_queues(void);


#endif /* __WRC_H__ */
