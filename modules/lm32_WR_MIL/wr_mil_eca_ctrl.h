#ifndef WR_MIL_ECA_CTRL_H_
#define WR_MIL_ECA_CTRL_H_

#include "wr_mil_value64bit.h"

typedef struct
{
  volatile uint32_t *pECACtrl;
  volatile uint32_t *pECATimeHi;
  volatile uint32_t *pECATimeLo;
} ECACtrl_t;

typedef struct 
{
  uint32_t channels_get;                // 0x00  //ro,  8 b, Number of channels implemented by the ECA, including the internal IO channel #0
  uint32_t search_capacity_get;         // 0x04  //ro, 16 b, Total number of search table entries per active page
  uint32_t walker_capacity_get;         // 0x08  //ro, 16 b, Total number of walker table entries per active page
  uint32_t latency_get;                 // 0x0c  //ro, 32 b, Delay in ticks (typically nanoseconds) between an event's arrival at the ECA and its earliest possible execution as an action
  uint32_t offset_bits_get;             // 0x10  //ro,  8 b, Actions scheduled for execution with a delay in ticks exceeding offset_bits are executed early
  uint32_t flip_active_owr;             // 0x14  //wo,  1 b, Flip the active search and walker tables with the inactive tables
  uint32_t time_hi_get;                 // 0x18  //ro, 32 b, Ticks (nanoseconds) since Jan 1, 1970 (high word)
  uint32_t time_lo_get;                 // 0x1c  //ro, 32 b, Ticks (nanoseconds) since Jan 1, 1970 (low word)
  uint32_t search_select_rw;            // 0x20  //rw, 16 b, Read/write this record in the inactive search table
  uint32_t search_rw_first_rw;          // 0x24  //rw, 16 b, Scratch register to be written to search_ro_first
  uint32_t search_rw_event_hi_rw;       // 0x28  //rw, 32 b, Scratch register to be written to search_ro_event_hi
  uint32_t search_rw_event_lo_rw;       // 0x2c  //rw, 32 b, Scratch register to be written to search_ro_event_lo
  uint32_t search_write_owr;            // 0x30  //wo,  1 b, Store the scratch registers to the inactive search table record search_select
  uint32_t search_ro_first_get;         // 0x34  //ro, 16 b, The first walker entry to execute if an event matches this record in the search table
  uint32_t search_ro_event_hi_get;      // 0x38  //ro, 32 b, Event IDs greater than or equal to this value match this search table record (high word)
  uint32_t search_ro_event_lo_get;      // 0x3c  //ro, 32 b, Event IDs greater than or equal to this value match this search table record (low word)
  uint32_t walker_select_rw;            // 0x40  //rw, 16 b, Read/write this record in the inactive walker table
  uint32_t walker_rw_next_rw;           // 0x44  //rw, 16 b, Scratch register to be written to walker_ro_next
  uint32_t walker_rw_offset_hi_rw;      // 0x48  //rw, 32 b, Scratch register to be written to walker_ro_offset_hi
  uint32_t walker_rw_offset_lo_rw;      // 0x4c  //rw, 32 b, Scratch register to be written to walker_ro_offset_lo
  uint32_t walker_rw_tag_rw;            // 0x50  //rw, 32 b, Scratch register to be written to walker_ro_tag
  uint32_t walker_rw_flags_rw;          // 0x54  //rw,  4 b, Scratch register to be written to walker_ro_flags
  uint32_t walker_rw_channel_rw;        // 0x58  //rw,  8 b, Scratch register to be written to walker_ro_channel
  uint32_t walker_rw_num_rw;            // 0x5c  //rw,  8 b, Scratch register to be written to walker_ro_num
  uint32_t walker_write_owr;            // 0x60  //wo,  1 b, Store the scratch registers to the inactive walker table record walker_select
  uint32_t walker_ro_next_get;          // 0x64  //ro, 16 b, The next walker entry to execute after this record (0xffff = end of list)
  uint32_t walker_ro_offset_hi_get;     // 0x68  //ro, 32 b, The resulting action's deadline is the event timestamp plus this offset (high word)
  uint32_t walker_ro_offset_lo_get;     // 0x6c  //ro, 32 b, The resulting action's deadline is the event timestamp plus this offset (low word)
  uint32_t walker_ro_tag_get;           // 0x70  //ro, 32 b, The resulting actions's tag
  uint32_t walker_ro_flags_get;         // 0x74  //ro,  4 b, Execute the resulting action even if it suffers from the errors set in this flag register
  uint32_t walker_ro_channel_get;       // 0x78  //ro,  8 b, The channel to which the resulting action will be sent
  uint32_t walker_ro_num_get;           // 0x7c  //ro,  8 b, The subchannel to which the resulting action will be sent
  uint32_t channel_select_rw;           // 0x80  //rw,  8 b, Read/clear this channel
  uint32_t channel_num_select_rw;       // 0x84  //rw,  8 b, Read/clear this subchannel
  uint32_t channel_code_select_rw;      // 0x88  //rw,  2 b, Read/clear this error condition (0=late, 1=early, 2=conflict, 3=delayed)
  uint32_t channel_type_get;            // 0x90  //ro, 32 b, Type of the selected channel (0=io, 1=linux, 2=wbm, ...)
  uint32_t channel_max_num_get;         // 0x94  //ro,  8 b, Total number of subchannels supported by the selected channel
  uint32_t channel_capacity_get;        // 0x98  //ro, 16 b, Total number of actions which may be enqueued by the selected channel at a time
  uint32_t channel_msi_set_enable_owr;  // 0x9c  //wo,  1 b, Turn on/off MSI messages for the selected channel
  uint32_t channel_msi_get_enable_get;  // 0xa0  //ro,  1 b, Check if MSI messages are enabled for the selected channel
  uint32_t channel_msi_set_target_owr;  // 0xa4  //wo, 32 b, Set the destination MSI address for the selected channel (only possible while it has MSIs disabled)
  uint32_t channel_msi_get_target_get;  // 0xa8  //ro, 32 b, Get the destination MSI address for the selected channel
  uint32_t channel_mostfull_ack_get;    // 0xac  //ro, 32 b, Read the selected channel's fill status (used_now<<16 | used_most), MSI=(6<<16) will be sent if used_most changes
  uint32_t channel_mostfull_clear_get;  // 0xb0  //ro, 32 b, Read and clear the selected channel's fill status (used_now<<16 | used_most), MSI=(6<<16) will be sent if used_most changes
  uint32_t channel_valid_count_get;     // 0xb4  //ro, 32 b, Read and clear the number of actions output by the selected subchannel, MSI=(4<<16|num) will be sent when the count becomes non-zero
  uint32_t channel_overflow_count_get;  // 0xb8  //ro, 32 b, Read and clear the number of actions which could not be enqueued to the selected full channel which were destined for the selected subchannel, MSI=(5<<16|num) will be sent when the count becomes non-zero
  uint32_t channel_failed_count_get;    // 0xbc  //ro, 32 b, Read and clear the number of actions with the selected error code which were destined for the selected subchannel, MSI=(code<<16|num) will be sent when the count becomes non-zero
  uint32_t channel_event_id_hi_get;     // 0xc0  //ro, 32 b, The event ID of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (high word)
  uint32_t channel_event_id_lo_get;     // 0xc4  //ro, 32 b, The event ID of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (low word)
  uint32_t channel_param_hi_get;        // 0xc8  //ro, 32 b, The parameter of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (high word)
  uint32_t channel_param_lo_get;        // 0xcc  //ro, 32 b, The parameter of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (low word)
  uint32_t channel_tag_get;             // 0xd0  //ro, 32 b, The tag of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read
  uint32_t channel_tef_get;             // 0xd4  //ro, 32 b, The TEF of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read
  uint32_t channel_deadline_hi_get;     // 0xd8  //ro, 32 b, The deadline of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (high word)
  uint32_t channel_deadline_lo_get;     // 0xdc  //ro, 32 b, The deadline of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (low word)
  uint32_t channel_executed_hi_get;     // 0xe0  //ro, 32 b, The actual execution time of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (high word)
  uint32_t channel_executed_lo_get;     // 0xe4  //ro, 32 b, The actual execution time of the first action with the selected error code on the selected subchannel, cleared when channel_failed_count is read (low word)
} ECARegs;

volatile ECARegs *ECACtrl_init(uint32_t *device_addr);
void ECACtrl_getTAI(volatile ECARegs *eca, TAI_t *tai);

#endif
