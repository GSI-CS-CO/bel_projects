class prio (object):
   ctrl = {
   'reset_owr'          : 0x000, # wo,          1 b, Resets the Priority Queue
   'mode_get'           : 0x004, # ro,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   'mode_clr'           : 0x008, # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   'mode_set'           : 0x00c, # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   'clear_owr'          : 0x010, # wo,          1 b, Clears counters and status
   'st_full_get'        : 0x014, # ro, g_channels b, Channel Full flag (n..0) 
   'st_late_get'        : 0x018, # ro,          1 b, Late message detected
   'ebm_adr_rw'         : 0x01c, # rw,         32 b, Etherbone Master address
   'eca_adr_rw'         : 0x020, # rw,         32 b, Event Condition Action Unit address
   'tx_max_msgs_rw'     : 0x024, # rw,          8 b, Max msgs per packet
   'tx_max_wait_rw'     : 0x028, # rw,         32 b, Max wait time for non empty packet
   'tx_rate_limit_rw'   : 0x02c, # rw,         32 b, Max msgs per milliseconds
   'offs_late_rw_0'     : 0x030, # rw,         32 b, Time offset before message is late
   'offs_late_rw_1'     : 0x034, # rw,         32 b, Time offset before message is late
   'cnt_late_get'       : 0x038, # ro,         32 b, Sum of all late messages
   'ts_late_get_0'      : 0x03c, # ro,         32 b, First late Timestamp
   'ts_late_get_1'      : 0x040, # ro,         32 b, First late Timestamp
   'cnt_out_all_get_0'  : 0x044, # ro,         32 b, Sum of all outgoing messages
   'cnt_out_all_get_1'  : 0x048, # ro,         32 b, Sum of all outgoing messages
   'ch_sel_rw'          : 0x100, # rw,          4 b, Channel select
   'cnt_out_get'        : 0x104, # ro,         32 b, Outgoing messages per Channel
   'cnt_in_get'         : 0x108, # ro,         32 b, Incoming messages per Channel
   }
   ctrl_reverse = {
   0x000 : 'reset_owr', # wo,          1 b, Resets the Priority Queue
   0x004 : 'mode_get', # ro,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   0x008 : 'mode_clr', # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   0x00c : 'mode_set', # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   0x010 : 'clear_owr', # wo,          1 b, Clears counters and status
   0x014 : 'st_full_get', # ro, g_channels b, Channel Full flag (n..0) 
   0x018 : 'st_late_get', # ro,          1 b, Late message detected
   0x01c : 'ebm_adr_rw', # rw,         32 b, Etherbone Master address
   0x020 : 'eca_adr_rw', # rw,         32 b, Event Condition Action Unit address
   0x024 : 'tx_max_msgs_rw', # rw,          8 b, Max msgs per packet
   0x028 : 'tx_max_wait_rw', # rw,         32 b, Max wait time for non empty packet
   0x02c : 'tx_rate_limit_rw', # rw,         32 b, Max msgs per milliseconds
   0x030 : 'offs_late_rw_0', # rw,         32 b, Time offset before message is late
   0x034 : 'offs_late_rw_1', # rw,         32 b, Time offset before message is late
   0x038 : 'cnt_late_get', # ro,         32 b, Sum of all late messages
   0x03c : 'ts_late_get_0', # ro,         32 b, First late Timestamp
   0x040 : 'ts_late_get_1', # ro,         32 b, First late Timestamp
   0x044 : 'cnt_out_all_get_0', # ro,         32 b, Sum of all outgoing messages
   0x048 : 'cnt_out_all_get_1', # ro,         32 b, Sum of all outgoing messages
   0x100 : 'ch_sel_rw', # rw,          4 b, Channel select
   0x104 : 'cnt_out_get', # ro,         32 b, Outgoing messages per Channel
   0x108 : 'cnt_in_get', # ro,         32 b, Incoming messages per Channel
   }
