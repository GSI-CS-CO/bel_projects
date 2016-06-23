class prio (object):
  ifname = 'ctrl'

  adrDict = {
  'reset_owr'         : 0x00, # wo,          1 b, Resets the Priority Queue
  'mode_get'          : 0x04, # ro,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  'mode_clr'          : 0x08, # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  'mode_set'          : 0x0c, # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  'clear_owr'         : 0x10, # wo,          1 b, Clears counters and status
  'st_full_get'       : 0x14, # ro, g_channels b, Channel Full flag (n..0) 
  'st_late_get'       : 0x18, # ro,          1 b, Late message detected
  'ebm_adr_rw'        : 0x1c, # rw,         32 b, Etherbone Master address
  'eca_adr_rw'        : 0x20, # rw,         32 b, Event Condition Action Unit address
  'tx_max_msgs_rw'    : 0x24, # rw,          8 b, Max msgs per packet
  'tx_max_wait_rw'    : 0x28, # rw,         32 b, Max wait time for non empty packet
  'tx_rate_limit_rw'  : 0x2c, # rw,         32 b, Max msgs per milliseconds
  'offs_late_rw_0'    : 0x30, # rw,         32 b, Time offset before message is late
  'offs_late_rw_1'    : 0x34, # rw,         32 b, Time offset before message is late
  'cnt_late_get'      : 0x38, # ro,         32 b, Sum of all late messages
  'ts_late_get_0'     : 0x3c, # ro,         32 b, First late Timestamp
  'ts_late_get_1'     : 0x40, # ro,         32 b, First late Timestamp
  'cnt_out_all_get_0' : 0x44, # ro,         32 b, Sum of all outgoing messages
  'cnt_out_all_get_1' : 0x48, # ro,         32 b, Sum of all outgoing messages
  }

  adrDict_reverse = {
  0x00  : 'reset_owr', # wo,          1 b, Resets the Priority Queue
  0x04  : 'mode_get', # ro,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  0x08  : 'mode_clr', # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  0x0c  : 'mode_set', # wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  0x10  : 'clear_owr', # wo,          1 b, Clears counters and status
  0x14  : 'st_full_get', # ro, g_channels b, Channel Full flag (n..0) 
  0x18  : 'st_late_get', # ro,          1 b, Late message detected
  0x1c  : 'ebm_adr_rw', # rw,         32 b, Etherbone Master address
  0x20  : 'eca_adr_rw', # rw,         32 b, Event Condition Action Unit address
  0x24  : 'tx_max_msgs_rw', # rw,          8 b, Max msgs per packet
  0x28  : 'tx_max_wait_rw', # rw,         32 b, Max wait time for non empty packet
  0x2c  : 'tx_rate_limit_rw', # rw,         32 b, Max msgs per milliseconds
  0x30  : 'offs_late_rw_0', # rw,         32 b, Time offset before message is late
  0x34  : 'offs_late_rw_1', # rw,         32 b, Time offset before message is late
  0x38  : 'cnt_late_get', # ro,         32 b, Sum of all late messages
  0x3c  : 'ts_late_get_0', # ro,         32 b, First late Timestamp
  0x40  : 'ts_late_get_1', # ro,         32 b, First late Timestamp
  0x44  : 'cnt_out_all_get_0', # ro,         32 b, Sum of all outgoing messages
  0x48  : 'cnt_out_all_get_1', # ro,         32 b, Sum of all outgoing messages
  }

  flagDict = {
  'reset'         : ['wp', 1], # Resets the Priority Queue
  'mode'          : ['rwa', 3], # b2: Time limit, b1: Msg limit, b0 enable
  'clear'         : ['wp', 1], # Clears counters and status
  'st_full'       : ['rd', 8], # Channel Full flag (n..0) 
  'st_late'       : ['rd', 1], # Late message detected
  'ebm_adr'       : ['rw', 32], # Etherbone Master address
  'eca_adr'       : ['rw', 32], # Event Condition Action Unit address
  'tx_max_msgs'   : ['rw', 8], # Max msgs per packet
  'tx_max_wait'   : ['rw', 32], # Max wait time for non empty packet
  'tx_rate_limit' : ['rw', 32], # Max msgs per milliseconds
  'offs_late'     : ['rw', 64], # Time offset before message is late
  'cnt_late'      : ['rd', 32], # Sum of all late messages
  'ts_late'       : ['rd', 64], # First late Timestamp
  'cnt_out_all'   : ['rd', 64], # Sum of all outgoing messages
  }

  valDict = {
  'reset'         : 0, # Resets the Priority Queue
  'mode'          : 0, # b2: Time limit, b1: Msg limit, b0 enable
  'clear'         : 0, # Clears counters and status
  'st_full'       : 0, # Channel Full flag (n..0) 
  'st_late'       : 0, # Late message detected
  'ebm_adr'       : 0, # Etherbone Master address
  'eca_adr'       : 0, # Event Condition Action Unit address
  'tx_max_msgs'   : 0, # Max msgs per packet
  'tx_max_wait'   : 0, # Max wait time for non empty packet
  'tx_rate_limit' : 0, # Max msgs per milliseconds
  'offs_late'     : 0, # Time offset before message is late
  'cnt_late'      : 0, # Sum of all late messages
  'ts_late'       : 0, # First late Timestamp
  'cnt_out_all'   : 0, # Sum of all outgoing messages
  }
