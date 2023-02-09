#ifndef __BURST_GENERATION_H__
#define __BURST_GENERATION_H__

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"
#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../common-libs/include/common-defs.h" // COMMON_STATUS_OK

/* shared memory map for communication via Wishbone  */
#include <burstgen_shared_mmap.h>        // autogenerated upon building firmware

/* includes common functions */
#include "fg.h" // included in cb.h
#include "cb.h" // add_msg()

typedef uint32_t status_t;

#define toU64(hi32, lo32, r64) { r64 = ((uint64_t)(hi32)) << 32; r64 |= (lo32); }
#define hiU32(u64)  ((uint32_t)(((u64) >> 32) & 0xFFFFFFFF))
#define loU32(u64)  ((uint32_t)((u64) & 0xFFFFFFFF))

#define STATUS_OK        COMMON_STATUS_OK
#define STATUS_ERR       COMMON_STATUS_ERROR
#define STATUS_IDLE      20
#define STATUS_NOT_READY 21
#define STATUS_DISABLED  22

const unsigned char errMsgEcaMsi[] = {"Cannot en/disable ECA MSI path to mailbox.\0"};

#define NWORDS            2048  // WARNING: don't exceed 'shared size'! A value of 1024 already requires 8k!
extern uint32_t*       _startshared[];

/* definitions of ECA channels */
#define ECAQMAX           4     // max number of ECA channels in the system
#define ECACHANNELFORLM32 2     // the id of an ECA channel for embedded CPU

/* definitions of buffers in shared memory */
#define BG_SHARED_BEGIN         COMMON_SHARED_END
#define BG_SHARED_MB_SLOT_LM32  BG_SHARED_BEGIN + 0x04UL  // Mailbox slot for LM32
#define BG_SHARED_MB_SLOT_HOST  BG_SHARED_BEGIN + 0x0CUL  // Mailbox slot for the host
#define BG_SHARED_CMD           COMMON_SHARED_CMD         // Command buffer
#define BG_SHARED_INPUT         BG_SHARED_BEGIN + 0x20UL  // Command argument buffer
#define BG_SHARED_END           BG_SHARED_BEGIN + 0x40UL  // End of the shared memory
#define MB_SLOT_CFG_FREE  0xFFFFFFFFUL

/* id number to identify the LM32 firmware for burst generator */
#define BG_FW_ID          0xb2b2b2b2UL
#define BG_FW_VERSION     0x000100   // 0xMMmmRR, M-major, m-minor, R-revision
#define BG_TAG            "bg"

#define N_BURSTS          17    // maximum number of bursts can be generated, but bursts 1..N_BURSTS-1 are used

enum BURST_INFO {                // burst info fields
  INFO_ID,                        // burst ID
  INFO_IO_TYPE,                   // IO type
  INFO_IO_IDX,                    // IO index
  INFO_START_EVT_H32,             // start event ID, high32
  INFO_START_EVT_L32,             // start event ID, low32
  INFO_STOP_EVT_H32,              // stop event ID, high32
  INFO_STOP_EVT_L32,              // stop event ID, low32
  INFO_LOOPS_H32,                 // loops/cycles, high32
  INFO_LOOPS_L32,                 // loops/cycles, low32
  INFO_ACTION_H32,                // action time, high32
  INFO_ACTION_L32,                // action time, low32
  INFO_FLAG,                      // burst flag
  N_BURST_INFO
};

/* user commands for the burst generator */
#define CMD_SHOW_ALL      0x21UL
#define CMD_GET_PARAM     0x22UL
#define CMD_GET_CYCLE     0x23UL
#define CMD_LS_BURST      0x24UL  // list burst (ids or burst info)
#define CMD_MK_BURST      0x25UL  // declare new burst
#define CMD_RM_BURST      0x26UL  // remove burst
#define CMD_DE_BURST      0x27UL  // dis/enable burst
#define CMD_RD_MSI_ECPU   0x30UL
#define CMD_RD_ECPU_CHAN  0x31UL
#define CMD_RD_ECPU_QUEUE 0x32UL
#define CMD_LS_FW_ID      0x33UL   // list the firmware id
#define CMD_MASK          0xFFFFUL // command mask

#define CMD_DIAG_TOGGLE_MEASUREMENT        0x40 // toggle diagnostic measurements
#define CMD_DIAG_PRINT_MSI_HANDLE_DURATION 0x41 // print the elapsed time to handle MSIs
#define CMD_DIAG_PRINT_IO_EVENT_CTRL_CFG   0x42 // print IO event control and configuration table
#define CMD_DIAG_PRINT_TASK_INTERVAL       0x43 // print the task interval

/* offsets of arguments in the user commands */
enum ARGS_CMD_GET_PARAM {
  GET_PARAM_ID,           // burst ID
  GET_PARAM_SETUP,        // timer period for burst burst head, ns
  GET_PARAM_N_CONDITION,  // number of conditions in a block
  GET_PARAM_PERIOD,       // block period, ns
  GET_PARAM_FLAG,         // burst flag
  GET_PARAM_VERBOSE,      // verbose
  N_GET_PARAM
};

enum ARGS_CMD_GET_CYCLE {
  GET_CYCLE_ID,           // burst ID
  GET_CYCLE_N_CYCLE_H32,  // number of block cycles, high32
  GET_CYCLE_N_CYCLE_L32,  // number of block cycles, low32
  GET_CYCLE_VERBOSE,      // verbose
  N_GET_CYCLE
};

enum ARGS_CMD_MK_BURST {
  MK_BURST_ID,            // burst ID
  MK_BURST_IO,            // target IO (io_type << 16 | io_index)
  MK_BURST_START_EVT_H32, // start event ID, high32
  MK_BURST_START_EVT_L32, // start event ID, low32
  MK_BURST_STOP_EVT_H32,  // stop event ID, high32
  MK_BURST_STOP_EVT_L32,  // stop event ID, low32
  MK_BURST_VERBOSE,       // verbose
  N_MK_BURST
};

/* definitions of timing messages & ECA actions */
#define ECA_FG_MOSTFULL   0x00060000UL  // ECA mostfull flag
#define ECA_FG_OVERFLOW   0x00050000UL  // ECA overflow flag (overflow cnt)
#define ECA_FG_VALID      0x00040000UL  // ECA valid flag (valid cnt)
#define ECA_FG_DELAYED    0x00030000UL  // ECA delayed flag (failed cnt)
#define ECA_FG_CONFLICT   0x00020000UL  // ECA conflict (failed cnt)
#define ECA_FG_EARLY      0x00010000UL  // ECA early (failed cnt)
#define ECA_FG_LATE       0x00000000UL  // ECA late (failed cnt)
#define ECA_FG_MASK       0x000F0000UL  // flag mask

#define MY_ACT_TAG        BG_FW_ID      // ECA actions tagged for me
#define IO_CYC_START      0x00009910UL  // start the IO pulse cycle
#define IO_CYC_STOP       0x00009900UL  // stop the IO pulse cycle
#define EVTNO_MIL         0x00000FF0UL  // event numbers used for MIL based system (0..255)
#define EVTNO_WR          0x0000F9F0UL  // event numbers used for WR based system (256..3999)
#define EVTNO_INTERN      0x0000FFF0UL  // event numbers for internal use (4000...4095)
#define EVTNO_MASK        0x0000FFF0UL  // event number mask

#define LEN_TIM_MSG       0x8           // length of timing message in bytes
#define EVT_ID_IO_H32     0x0000FCA0UL  // event id of timing message for IO actions (hi32)
#define EVT_ID_IO_L32     0x00000000UL  // event id of timing message for IO actions (lo32)
#define EVT_MASK_IO       0xFFFFFFFF00000000ULL

/* MSI sender offsets */
#define MSI_OFFS_ECA      0x00UL        // ECA
#define MSI_OFFS_HOST     0x10UL        // host
#define MSI_OFFS_MASK     0xFFUL        // offset mask

/* ECA conditions for bursts at IO channel */
typedef struct {
  uint32_t nConditions; // number of ECA conditions
  uint32_t tOffset;     // and their offsets
} EcaIoRules_t;

/* ECA feeder */
typedef struct {
  uint64_t id;          // timing msg id
  int64_t cycle;        // cycle duration in ns
  uint64_t period;      // period of timing msg
  uint64_t deadline;    // deadline of timing msg
  void (*f)(void);      // feeder function
} Feeder_t;

/* declaration for the task scheduler, derived from function generator fw */
#define INTERVAL_60S      60000000000ULL
#define INTERVAL_1000MS   1000000000ULL
#define INTERVAL_2000MS   2000000000ULL
#define INTERVAL_100MS    100000000ULL
#define INTERVAL_84MS     84000000ULL
#define INTERVAL_10MS     10000000ULL
#define INTERVAL_5MS      5000000ULL
#define INTERVAL_1MS      1000000ULL
#define INTERVAL_500US    500000ULL
#define INTERVAL_200US    200000ULL
#define INTERVAL_150US    150000ULL
#define INTERVAL_100US    100000ULL
#define INTERVAL_10US     10000ULL
#define ALWAYS            0ULL

#define SEC_SCALE         1000000000ULL
#define MS_SCALE          1000000ULL
#define US_SCALE          1000ULL

#define CTL_DIS           0x0000UL
#define CTL_EN            0x0001UL
#define CTL_VALID         0x8000UL

typedef struct {
  int state;
  uint32_t flag;         /* control flag: CTL_VALID & CTL_EN, CTL_DIS */
  uint8_t  io_type;      /* IO port type: 0=GPIO, 1=LVDS, 2=FIXED (io_control_regs.h */
  uint8_t  io_index;     /* IO port index, info_index of t_io_mapping_table in monster_pkg.vhd */
  uint64_t trigger;      /* trigger event ID */
  uint64_t toggle;       /* toggling event ID */
  uint64_t setup;        /* time period needed for burst head, ns */
  int64_t  cycle;        /* handler-specific: number of cycles */
  uint64_t period;       /* handler-specific: period in ns */
  uint64_t deadline;     /* handler-specific: deadline */
  uint64_t interval;     /* interval of the task */
  uint64_t lasttick;     /* when was the task ran last */
  uint64_t action;       /* when was the IO action trigged */
  uint64_t failed;       /* task failed timestamp */
  int (*func)(int);     /* pointer to the function of the task */
} Task_t;

/* burst trigger/toggle control */
typedef struct {
  uint64_t deadline;
  uint32_t bursts;    /* burst indices, bitwise, bit0: burst idx 1 */
} Control_t;

typedef struct {
  uint64_t id;        /* event ID */
  uint32_t bursts;    /* burst indices, bitwise, bit0: burst idx 1 */
} Config_t;

#define N_CONFIGS         64 /* configuration table entries, limited by HW */

extern struct msi remove_msg(volatile struct message_buffer *mb, int queue);
extern int add_msg(volatile struct message_buffer *mb, int queue, struct msi m);
extern int has_msg(volatile struct message_buffer *mb, int queue);

#endif
