/*
 * This work is part of the White Rabbit project
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#ifndef CONFIG_TARGET_GSI_DEVICE
//#include <netconsole.h>
#endif

#ifndef __BOARD_WRC_H
#define __BOARD_WRC_H
/*
 * This is meant to be automatically included by the Makefile,
 * when wrpc-sw is build for wrc (node) -- as opposed to wrs (switch)
 */

#ifdef CONFIG_ARCH_RISCV
#define DEV_BASE	0x100000
#elif defined CONFIG_ARCH_LM32
#define DEV_BASE	0x80040000
#else
#error (Wrong Arch!)
#endif

/* Fixed base addresses */
#define BASE_MINIC		(DEV_BASE + 0x000)
#define BASE_EP			(DEV_BASE + 0x100)
#define BASE_SOFTPLL		(DEV_BASE + 0x200)
#define BASE_PPS_GEN 		(DEV_BASE + 0x300)
#define BASE_SYSCON		(DEV_BASE + 0x400)
#define BASE_UART		(DEV_BASE + 0x500)
#define BASE_ONEWIRE		(DEV_BASE + 0x600)
#define BASE_WDIAGS_PRIV       	(DEV_BASE + 0x900)
#define BASE_ETHERBONE_CFG	(DEV_BASE + 0x8000)

/* Board-specific parameters */
#define TICS_PER_SECOND 1000

/* WR Core system/CPU clock frequency in Hz */
#define CPU_CLOCK 62500000ULL

/* WR Reference clock period (picoseconds) and frequency (Hz) */
#ifdef CONFIG_TARGET_GENERIC_PHY_16BIT
#  define NS_PER_CLOCK 16
#  define REF_CLOCK_PERIOD_PS 16000
#  define REF_CLOCK_FREQ_HZ 62500000
#else
#  define NS_PER_CLOCK 8
#  define REF_CLOCK_PERIOD_PS 8000
#  define REF_CLOCK_FREQ_HZ 125000000
#endif

/* Maximum number of simultaneously created sockets */
#define NET_MAX_SOCKETS 12

/* Socket buffer size, determines the max. RX packet size */
#define NET_MAX_SKBUF_SIZE 512

/* Number of auxillary clock channels - usually equal to the number of FMCs */
#define NUM_AUX_CLOCKS 1

/* spll parameter that are board-specific */
#ifdef CONFIG_TARGET_GENERIC_PHY_16BIT
#  define BOARD_DIVIDE_DMTD_CLOCKS	0
#else
#  define BOARD_DIVIDE_DMTD_CLOCKS	1
#endif
#define BOARD_MAX_CHAN_REF		1
#define BOARD_MAX_CHAN_AUX		2
#define BOARD_MAX_PTRACKERS		1

#undef CONFIG_DISALLOW_LONG_DIVISION

#define BOARD_USE_EVENTS 0

#define BOARD_MAX_CONSOLE_DEVICES (1 + HAS_NETCONSOLE + HAS_PUTS_SYSLOG)

#define CONSOLE_UART_BAUDRATE 115200

#define FMC_EEPROM_ADR 0x50

#define SDBFS_REC 5

#define EEPROM_STORAGE 0

#if 0
void sdb_find_devices(void);
void sdb_print_devices(void);
#endif

#endif /* __BOARD_WRC_H */
